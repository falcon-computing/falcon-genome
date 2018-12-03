### Jenkins Build-Status
AWS [![Build Status](http://us-1.falcon-computing.com:8080/buildStatus/icon?job=Falcon-Build-Falcon-genome)](http://us-1.falcon-computing.com:8080/job/Falcon-Build-Falcon-genome/)
HWC [![Build Status](http://us-1.falcon-computing.com:8080/buildStatus/icon?job=HWC-Falcon-Genome)](http://us-1.falcon-computing.com:8080/view/HWC/job/HWC-Falcon-Genome/)
Local [![Build Status](http://us-1.falcon-computing.com:8080/buildStatus/icon?job=Falcon-Build-Falcon-Genome-Internal)](http://us-1.falcon-computing.com:8080/job/Falcon-Build-Falcon-Genome-Internal/)
# Falcon Genome CLI
This project is the command line interface for the Falcon Accelerated Genomics Pipelines.
Test
### Build
1. Build and install
   ```
   mkdir Release; cd Release
   cmake -DCMAKE_BUILD_TYPE=Release ..
   make
   make test
   make install
   ```
   This will automatically install the `fcs-genome` binary to `~/.falcon-genome/fcs-genome/${version}/`. If the default install path needs to be changed, it can be done by adding `-DCMAKE_INSTALL_PREFIX=`

1. Choose which platform to deploy (aws, hwc, aliyun). This option mostly affect how the license is configured. If left blank, the default license mode is flexlm.
   ```
   mkdir Release_AWS; cd Release_AWS
   cmake -DCMAKE_BUILD_TYPE=Release -DDEPLOYMENT_DST=aws ..
   make
   make test
   make install
   ```
   Or:
   ```
   mkdir Release_HWC; cd Release_HWC
   cmake -DCMAKE_BUILD_TYPE=Release -DDEPLOYMENT_DST=hwc ..
   make
   make test
   make install
   ```

1. Build in Debug mode:
   ```
   mkdir Debug; cd Debug
   cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=`pwd`/install ..
   ```

