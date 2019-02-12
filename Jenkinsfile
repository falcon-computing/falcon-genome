//update1.0.15
pipeline {
agent {label 'merlin'}
    stages {
        stage ("build-aws-falcon-genome") {
            steps {
                 dir("ws-falcon-genome") {
                     checkout scm
//                    git branch: 'release', url: 'git@github.com:falcon-computing/falcon-genome.git'
                    sh "rm -rf release"
                    sh "mkdir release"
                    script { 
                    dir("release"){
//                        sh "rsync -av --exclude=.* /curr/limark/Code-latest/genome-release/build/aws/ /curr/limark/falcon2/"
//                        sh "rsync -av --exclude=.* /curr/limark/Code-latest/genome-release/build/common/ /curr/limark/falcon2/"
                        sh "source /curr/software/util/modules-tcl/init/bash"
                        version= sh(returnStdout: true, script: 'git describe --tag').trim()
                        sh "echo $version"
                        sh "module load sdx/17.4; cmake3 -DCMAKE_BUILD_TYPE=Release -DRELEASE_VERSION=$version -DDEPLOYMENT_DST=aws -DCMAKE_INSTALL_PREFIX=/curr/limark/falcon2/bin .."
                        sh "make -j 8"
                        sh "make test"
                        sh "make install"
//                        link = sh(returnStdout: true, script: 'cd /curr/limark/falcon2/bin; link=s3://fcs-cicd-test/release/aws/falcon-genome/fcs-genome; echo $link; echo $link > latest')
                        sh "cd ~/falcon2/bin; echo s3://fcs-cicd-test/release/aws/falcon-genome/fcs-genome-$version-aws > latest"
                        sh "cd /curr/limark/falcon2/bin; aws s3 cp fcs-genome s3://fcs-cicd-test/release/aws/falcon-genome/fcs-genome-$version-aws"
                        sh "cd /curr/limark/falcon2/bin; aws s3 cp latest s3://fcs-cicd-test/release/aws/falcon-genome/latest"
                        sh "cd /curr/limark/falcon2/bin; rm -f latest"                           
                      
                        }
                    }
                }
            }
        }
    }    
}
