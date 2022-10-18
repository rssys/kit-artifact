FROM ubuntu:16.04
RUN     apt-get update                                                  \ 
        && apt-get install -y git build-essential libncurses-dev        \   
                bison openssl libssl-dev dkms libelf-dev gawk flex      \
                libudev-dev libpci-dev libiberty-dev autoconf llvm      \
                gcc-4.8 gcc-5 bc
COPY build_kernel.sh /
RUN chmod +x /build_kernel.sh
ENTRYPOINT ["/build_kernel.sh"]