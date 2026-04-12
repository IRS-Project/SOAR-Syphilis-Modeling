FROM gcc:14 AS builder

WORKDIR /app
COPY epiworld.hpp syphilis.hpp main.cpp Makefile ./

RUN make OPENMP=1

FROM debian:trixie-slim

RUN apt-get update && apt-get install -y --no-install-recommends libgomp1 libstdc++6 python3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/main.o .
COPY summarize.py .

ENTRYPOINT ["python3", "summarize.py"]
