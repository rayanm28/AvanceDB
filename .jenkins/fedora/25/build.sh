sudo dnf install -y autoconf213 && \
export SHELL=/bin/bash && \
export PATH=/usr/lib64/ccache:$PATH && \
mkdir -p ${WORKSPACE}/.ccache && export CCACHE_DIR=${WORKSPACE}/.ccache && \
mkdir -p ${WORKSPACE}/.npm && export npm_config_cache=${WORKSPACE}/.npm && \
mkdir -p ${WORKSPACE}/.gem && export GEM_HOME=${WORKSPACE}/.gem && export PATH=$GEM_HOME/bin:$PATH && \
sudo couchdb -b && \
cd build && \
make -j4 &&
make -j4 test && \
sudo couchdb -d
