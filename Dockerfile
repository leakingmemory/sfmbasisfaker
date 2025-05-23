FROM docker.io/gentoo/portage AS portage

FROM docker.io/gentoo/stage3:musl AS builder-stage1
COPY --from=portage /var/db/repos/gentoo /var/db/repos/gentoo
RUN emerge --update --newuse --deep @world
RUN emerge app-eselect/eselect-repository
RUN emerge sys-apps/busybox
RUN eselect repository enable guru
RUN emerge --sync guru
RUN echo 'dev-cpp/cpprestsdk ~amd64' > /etc/portage/package.accept_keywords/cpprestsdk
RUN emerge cpprestsdk
RUN eselect repository add leakingmemory git https://github.com/leakingmemory/gentoo-overlay.git
RUN echo '>=dev-cpp/libsfmbasisapi-0.4.1 ~amd64' > /etc/portage/package.accept_keywords/libsfmbasisapi
RUN echo 'dev-cpp/libjjwtid ~amd64' > /etc/portage/package.accept_keywords/libjjwtid
RUN emerge --sync leakingmemory
RUN quickpkg "*/*"

# Base system install
RUN ROOT=/gentoo USE=build emerge -1 baselayout
RUN ROOT=/gentoo emerge -K sys-libs/musl
RUN ROOT=/gentoo emerge -K sys-apps/busybox
RUN ROOT=/gentoo emerge app-misc/ca-certificates
RUN ROOT=/gentoo emerge -K gcc

# App requirements
RUN ROOT=/gentoo emerge -K cpprestsdk
RUN ROOT=/gentoo emerge libsfmbasisapi
RUN ROOT=/gentoo emerge libjjwtid

RUN cd /gentoo/usr/bin && ln -s busybox sh

FROM builder-stage1 AS passwd
RUN groupadd -g 1000 sfmbasisfaker
RUN useradd -u 1000 -g 1000 sfmbasisfaker

# Build app
FROM builder-stage1 AS builder-stage2

RUN ROOT=/gentoo emerge app-misc/ca-certificates
RUN ROOT=/gentoo emerge -K app-alternatives/gzip binutils cmake make grep awk diffutils gawk
RUN ROOT=/gentoo emerge "<sys-kernel/linux-headers-5.15"

FROM scratch AS builder-stage3
COPY --from=builder-stage2 /gentoo /

# App install
COPY . /src
RUN mkdir /obj
WORKDIR /obj
RUN cmake /src
RUN make
RUN make install

FROM scratch AS builder-stage4
COPY --from=builder-stage1 /gentoo /

# Cheating
RUN rm -rfv /usr/lib/gcc/*/*/*san*
RUN rm -rfv /usr/lib/gcc/*/*/*.a
RUN rm -rfv /usr/lib/gcc/*/*/include
RUN rm -rfv /usr/lib/gcc/*/*/plugin
RUN rm -rfv /usr/include
RUN rm -rfv /usr/libexec
RUN rm -rfv /usr/share/doc
RUN rm -rfv /usr/x86_64-pc-linux-musl
RUN rm -rfv /var/db/pkg
RUN mv -v /usr/lib64/* /usr/lib

# Bragging
RUN ls /

# Cleanup
FROM scratch
COPY --from=builder-stage4 / /
COPY --from=builder-stage3 /usr/local/bin/sfmbasisfaker /usr/local/bin/sfmbasisfaker
COPY --from=passwd /etc/passwd /etc/passwd
COPY --from=passwd /etc/group /etc/group
COPY --from=passwd /etc/shadow /etc/shadow
COPY --from=passwd /home/sfmbasisfaker /home/sfmbasisfaker
USER sfmbasisfaker
EXPOSE 8080
VOLUME /home/sfmbasisfaker
ENTRYPOINT [ "/usr/local/bin/sfmbasisfaker" ]
CMD [ "/usr/local/bin/sfmbasisfaker" ]
