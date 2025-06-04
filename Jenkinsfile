pipeline {
    agent any
    options {
        skipStagesAfterUnstable()
    }
    stages {
        stage('Build') {
            steps {
                script {
                    def image = docker.build("docker.radiotube.org/sfmbasisfaker:${env.BRANCH_NAME}-${env.BUILD_ID}")
                    image.push()
                    image.push("latest")
                }
            }
        }
    }
}