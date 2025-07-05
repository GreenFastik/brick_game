FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
wget gnupg2 lsb-release \
      libncurses-dev ncurses-base ncurses-bin ncurses-term \
      build-essential pkg-config \
      check lcov cppcheck valgrind \
      texlive-latex-base texinfo doxygen \
    && wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key \
         | gpg --dearmor -o /usr/share/keyrings/llvm-archive-keyring.gpg \
    && echo "deb [signed-by=/usr/share/keyrings/llvm-archive-keyring.gpg] \
         http://apt.llvm.org/$(lsb_release -cs)/ llvm-toolchain-$(lsb_release -cs)-20 main" \
       > /etc/apt/sources.list.d/llvm-20.list \
    && apt-get update \
    && apt-get install -y clang-format-20 \
    && update-alternatives --install /usr/bin/clang-format clang-format \
         /usr/bin/clang-format-20 100 \
    && rm -rf /var/lib/apt/lists/*

ENV TERM=xterm
WORKDIR /app
COPY .clang-format .clang-format
COPY . .

CMD ["sh", "-c", "make clean && make check"]