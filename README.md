# Falcon Genome CLI
This project is the command line interface for the Falcon Accelerated Genomics Pipelines.

## Build
```
mkdir build; cd build
cmake -DCMAKE_INSTALL_PREFIX=`pwd`/install ..
make 
make test
make install
```

This will produce a binary `fcs-genome` in `./build/install` folder.

### Build configuration
1. Build in release mode
   ```
   mkdir Release; cd Release
   cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=`pwd`/install ..
   make install
   ```
   Or build in Debug mode:
   ```
   mkdir Debug; cd Debug
   cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=`pwd`/install ..
   ```

2. In release mode, choose which platform to deploy (aws, hwc, aliyun). This option mostly affect how the license is configured. If left blank, the default license mode is flexlm.
   ```
   cmake -DCMAKE_BUILD_TYPE=Release -DDEPLOYMENT_DST=aws ..
   cmake -DCMAKE_BUILD_TYPE=Release -DDEPLOYMENT_DST=hwc ..
   ```
