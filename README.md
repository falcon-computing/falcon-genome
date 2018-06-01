# Falcon Genome CLI
This project is the command line interface for the Falcon Accelerated Genomics Pipelines.

## Build
```
mkdir build; cd build
cmake ..
make 
make test
```

This will produce a binary `fcs-genome` in `./build` folder.

### Build configuration
1. Build in release or debug mode
   ```
   cmake -DCMAKE_BUILD_TYPE=Release ..
   cmake -DCMAKE_BUILD_TYPE=Debug ..
   ```

2. In release mode, choose which platform to deploy (aws, hwc, aliyun). This option mostly affect how the license is configured. If left blank, the default license mode is flexlm.
   ```
   cmake -DCMAKE_BUILD_TYPE=Release -DDEPLOYMENT_DST=aws ..
   ```
