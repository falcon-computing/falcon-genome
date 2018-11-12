//update1.0.14
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
                        sh "rsync -av --exclude=.* /curr/limark/test/genome-release/build/aws/ /curr/limark/falcon2/"
                        sh "rsync -av --exclude=.* /curr/limark/test/genome-release/build/common/ /curr/limark/falcon2/"
                        sh "source /curr/software/util/modules-tcl/init/bash"
                        version = sh(returnStdout: true, script: 'git describe --tag')
                        sh "version=$version+test"
                        sh "echo $version"
                        sh "module load sdx/17.4; cmake -DCMAKE_BUILD_TYPE=Release -DRELEASE_VERSION=v1.2.1-111-g561a333 -DDEPLOYMENT_DST=aws -DCMAKE_INSTALL_PREFIX=/curr/limark/falcon2/bin .."
//                        sh "make -j 8"
//                        sh "make install"
//                        sh "cd ~/falcon2/bin; mv fcs-genome fcs-genome-$version" 
//                        link = sh(returnStdout: true, script: 'cd /curr/limark/falcon2/bin; link=s3://fcs-cicd-test/release/aws/falcon-genome/fcs-genome; echo $link; echo $link > latest')
//                        sh "cd /curr/limark/falcon2/bin; aws s3 cp fcs-genome-$version s3://fcs-cicd-test/release/aws/falcon-genome/fcs-genome-$version"
 //                       sh "cd /curr/limark/falcon2/bin; aws s3 cp latest s3://fcs-cicd-test/release/aws/falcon-genome/latest"
 //                       sh "cd /curr/limark/falcon2/bin; rm -f latest"
                        }
                     }
                  }
               }    
            }
         }    
    
    post {
            always {

                emailext attachLog: true, body: "${currentBuild.currentResult}: Job ${env.JOB_NAME} build ${env.BUILD_NUMBER}\n More info at: ${env.BUILD_URL}console",
                    recipientProviders: [[$class: 'DevelopersRecipientProvider'], [$class: 'RequesterRecipientProvider']],
                    subject: "Jenkins Build ${currentBuild.currentResult}: Job ${env.JOB_NAME}",
                    to: 'udara@limarktech.com, roshantha@limarktech.com'

        }
    }
}

