#!/usr/bin/env python
from multiprocessing import Queue
from threading import Thread
import argparse
import getpass
import json
import logging
import os
import errno
import sys, getopt, signal
import sysv_ipc as ipc

class QueueManager(object):

  qin = None
  qout = None

  # thread pool for workers
  task_q = Queue()

  checkpoint_dir = os.path.expanduser("~")+'/.fcs-genome/checkpoints'

  def mkdir(self, path): 
    try:
      os.makedirs(path)
    except OSError as e:
      if e.errno != errno.EEXIST:
        raise

  def checkpoint(self, fname, data):
    path = self.checkpoint_dir + '/' + fname
    with open(path, 'w') as fout:
      fout.write(data)
    self.logger.info('saving checkpoint to {1}'.format(path))

  def worker(self):
    while True:
      item = self.task_q.get()
      self.worker_func(item)

  def __init__(self, **kargs):
    if 'name' in kargs:
      self.name = kargs['name'] 
    else:
      self.name = 'manager'

    self.checkpoint_dir += '/'+self.name.lower().replace(" ", "-")
    self.mkdir(self.checkpoint_dir)

    # pwd
    self.dir = os.path.dirname(os.path.realpath(__file__))

    # config logging
    log_format = "%(asctime)-15s %(levelname)s - %(message)s"

    if 'log_level' in kargs:
      log_level = kargs['log_level']
    else:
      log_level = logging.INFO

    if 'log_path' in kargs:
      logging.basicConfig(filename=kargs['log_path'], format=log_format, level=log_level)
    else:
      logging.basicConfig(format=log_format, level=log_level)

    self.logger = logging.getLogger(self.name)
    self.logger.info(self.dir)

    # Open input queue
    if 'qin_id' in kargs:
      self.logger.info('Opening input queue with name ' + str(kargs['qin_id']))
      self.qin = ipc.MessageQueue(kargs['qin_id'], flags=ipc.IPC_CREAT)
    else:
      raise ValueError('ID for the input queue must be specified')

    # Open output queue
    if 'qout_id' in kargs:
      self.logger.info('Opening output queue with name ' + str(kargs['qout_id']))
      # TODO: exception handling
      try:
        self.qout = ipc.MessageQueue(kargs['qout_id'], flags=0)
      except ipc.ExistentialError:
        raise ValueError('Cannot find output queue, exiting...')

    def handler(signum, frame):
      if self.qin:
        self.qin.remove()

    signal.signal(signal.SIGINT, handler)
    signal.signal(signal.SIGTERM, handler)

    num_workers = 4
    if 'num_workers' in kargs:
      num_workers = kargs['num_workers']

    # start workers
    for i in range(num_workers):
      t = Thread(target=self.worker)
      t.daemon = True
      t.start()

  def listen(self):
    self.logger.info("Start listen")
    while True:
      msg, t = self.qin.receive()
      self.logger.debug("Received a mssage")
      self.logger.debug(msg)
      self.task_q.put(msg)

