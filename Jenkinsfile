
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
                        sh "rsync -av --exclude=.* /curr/limark/Code-latest/genome-release/build/aws/ /curr/limark/falcon2/"
                        sh "rsync -av --exclude=.* /curr/limark/Code-latest/genome-release/build/common/ /curr/limark/falcon2/"
                        sh "source /curr/software/util/modules-tcl/init/bash"
                        version= sh(returnStdout: true, script: 'git describe --tag').trim()
                        sh "echo $version"
                        sh "module load sdx/17.4; cmake -DCMAKE_BUILD_TYPE=Release -DRELEASE_VERSION=$version -DDEPLOYMENT_DST=aws -DCMAKE_INSTALL_PREFIX=/curr/limark/falcon2/bin .."
                        sh "make -j 8"
                        sh "make install"
//                        link = sh(returnStdout: true, script: 'cd /curr/limark/falcon2/bin; link=s3://fcs-cicd-test/release/aws/falcon-genome/fcs-genome; echo $link; echo $link > latest')
                        sh "cd ~/falcon2/bin; echo s3://fcs-cicd-test/release/aws/falcon-genome/fcs-genome-$version > latest"
                        sh "cd /curr/limark/falcon2/bin; aws s3 cp fcs-genome s3://fcs-cicd-test/release/aws/falcon-genome/fcs-genome-$version"
                        sh "cd /curr/limark/falcon2/bin; aws s3 cp latest s3://fcs-cicd-test/release/aws/falcon-genome/latest"
                        sh "cd /curr/limark/falcon2/bin; rm -f latest"
                            sh "cp /curr/limark/Code-latest/genome-release/get_latest.sh ."
//                            pwd = sh(returnStdout: true, script: 'pwd; echo $pwd') 
                            sh './get_latest.sh aws/falcon-genome'
                            sh "ls -ltrh"
                            sh "mv fcs-genome-$version fcs-genome;cp fcs-genome ~/falcon2/bin"
                            sh "pwd"
                            sh './get_latest.sh aws/bwa-flow'
                            sh "cp bwa-flow ~/falcon2/tools/bin"
                            sh './get_latest.sh aws/blaze'
                            sh 'tar zxf *.tgz -C ~/falcon2'
                            sh './get_latest.sh aws/gatk3'
                            sh 'cp GATK3.jar ~/falcon2/tools/package'
                            sh './get_latest.sh aws/gatk4'
                            sh 'cp GATK4.jar ~/falcon2/tools/package'
                        script {
                           sh "cd ~/;tar zcf falcon-genome-Internal-aws.tgz falcon2/"
//                            sh "tar zcf falcon-genome-aws.tgz  /curr/limark/falcon2/"
                            link = sh(returnStdout: true, script: 'link=s3://fcs-cicd-test/release/aws/falcon-genome-Internal-aws.tgz; echo $link; echo $link > latest')
                            sh 'cd ~/;aws s3 cp falcon-genome-Internal-aws.tgz s3://fcs-cicd-test/release/aws/falcon-genome-Internal-aws.tgz'
//                            sh "aws s3 cp falcon-genome-${Release_Version}.tgz $link"
                            sh "cd ~/;aws s3 cp latest s3://fcs-cicd-test/release/aws/latest"
                            sh "rm -f latest"
                            }
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
