#!/bin/groovy

properties([[$class: 'jenkins.model.BuildDiscarderProperty',
            strategy: [$class: 'LogRotator',
                        daysToKeepStr: '10',
                        numToKeepStr: '100',
                        artifactNumToKeepStr: '10']]])

catchError() {
    def wspwd = "";
    
    node('master') {
        stage name:"Checkout", concurrency:1
        checkout scm
        sh """set -x
              git submodule foreach "git reset --hard || true" || true
              git reset --hard || true
              git submodule update --init || true
           """
    
        stash includes: '**, .git/', excludes: 'ws/**', name: 'source', useDefaultExcludes: false
        
        sh "env"
        
        wspwd = pwd()
    }
    
    String[] archs = ["windows.i386", "windows.x86_64", "linux.i386", "linux.x86_64", "apple.universal" ]
    def parallel_map = [:]
    
    for (int i = 0 ; i < archs.size(); ++i) {
        def x = archs.get(i)
        parallel_map["${x}"] = { 
            node('master') {
                ws(wspwd+"/ws/"+x) {
                    echo "Build ${x} in "+pwd()
                    sh 'chmod -R u+w .git || true' // fixes unstash overwrite bug ... #JENKINS-33126
                    unstash 'source'
                    sh """set -x
                          BARCH=--arch=c.${x}
                          if [ "\$(uname -s | tr "[:upper:]" "[:lower:]").\$(uname -m)" = "${x}" ] ; then
                              BARCH=
                          fi
                          PARAMS=
                          # if [[ "${env.BRANCH_NAME}" == PR-* ]] ; then
                          if [ "${env.BRANCH_NAME}" == "master" ] ; then
                              PARAMS=create_nightly
                          #elif [ "${env.BRANCH_NAME}" == "latest" ] ; then
                          #    PARAMS=create_release
                          fi
                          docker run --rm -u jenkins -v \$(pwd):/workdir \
                                                     -v ~/.ssh:/home/jenkins/.ssh \
                                                     -v ~/.ccache:/workdir/.ccache \
                                                     -v /srv/apache2/siedler25.org/nightly:/www \
                                                     -v /srv/backup/www/s25client:/archive \
                                                     --name "${env.BUILD_TAG}-${x}" \
                                                     ubuntu/crossbuild:precise -c \
                                                     "cd build && ./cmake.sh --prefix=. \$BARCH -DRTTR_USE_STATIC_BOOST=ON -DRTTR_PREFIX= -RTTR_LIBDIR=share/s25rttr && make \$PARAMS"
                       """
                    archive 's25rttr*.tar.bz2,s25rttr*.zip'
                }
            } 
        }
    }
    
    // mirror to launchpad step
    parallel_map["mirror"] = {
                node('master') {
                            sh """set -x
                                  mkdir -p ws
                                  pushd ws
                                  if [ ! -d s25client.git ] ; then
                                      git clone --mirror https://github.com/Return-To-The-Roots/s25client.git
                                      (cd s25client.git && git remote set-url --push origin git+ssh://git.launchpad.net/s25rttr)
                                  fi
                                  cd s25client.git
                                  git fetch -p origin
                                  git push --mirror
                            """
                            // todo: mirror submodules?
                }
    }
    
    /*
    parallel_map["upload-ppa"] = {
                node('master') {
                            ws(wspwd+"/ws/upload-ppa") {
                                        echo "Upload to PPA in "+pwd()
                                        sh 'chmod -R u+w .git || true' // fixes unstash overwrite bug ... #JENKINS-33126
                                        unstash 'source'
                                        sh """set -x
                                              if [ "${env.BRANCH_NAME}" == "master" ] || [ "${env.BRANCH_NAME}" == "latest" ] ; then
                                                    cd release
                                                    ./build_deb.sh ${env.BUILD_NUMBER} || exit 1
                                              fi
                                        """
                            }
                }
    }
    */
    
    stage name:"Building", concurrency:1
    parallel parallel_map
    
    stage name:"Publishing", concurrency:1
    node('master') {
                sh """set -x
                      alias ssh="ssh -o ForwardX11=no"
                      cd release
                      ./upload_urls.sh nightly
                      ./upload_urls.sh stable
                """
                archive 'release/changelog-*.txt,release/rapidshare-*.txt'
    }
}

node {
    step([$class: 'Mailer',
          notifyEveryUnstableBuild: true,
          recipients: "sf-team@siedler25.org",
          sendToIndividuals: true])
}
