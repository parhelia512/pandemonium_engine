ARG img_version
FROM pandemonium-fedora:${img_version}

ENV EMSCRIPTEN_VERSION=3.1.53

RUN git clone --branch ${EMSCRIPTEN_VERSION} --progress https://github.com/emscripten-core/emsdk && \
    emsdk/emsdk install ${EMSCRIPTEN_VERSION} && \
    emsdk/emsdk activate ${EMSCRIPTEN_VERSION}

CMD /bin/bash
