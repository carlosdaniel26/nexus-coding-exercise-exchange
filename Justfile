set shell := ["bash", "-uc"]

root := justfile_directory()
feed_url := "http://127.0.0.1:3000"

default:
    @just --list

# build the Rust feed and the Rust client
build:
    cd "{{root}}/services" && cargo build
    cd "{{root}}/exercise" && just build

# run the feed in the foreground (Ctrl-C to stop)
feed accounts="20" rate="2":
    #!/usr/bin/env bash
    set -euo pipefail
    cd "{{root}}/services"
    cargo build
    exec ./target/debug/services --start-feed --num-accounts {{accounts}} --rate {{rate}}

# boot the feed and the client together, then tear the feed down
demo n="20":
    #!/usr/bin/env bash
    set -euo pipefail
    if curl -sf {{feed_url}}/symbols >/dev/null 2>&1; then
        echo "port 3000 already serving a feed; stop it first" >&2
        exit 1
    fi
    cd "{{root}}/services"
    cargo build
    ./target/debug/services --start-feed --num-accounts 20 --rate 2 &
    feed_pid=$!
    trap 'kill "$feed_pid" 2>/dev/null || true' EXIT INT TERM
    until curl -sf {{feed_url}}/symbols >/dev/null; do sleep 0.2; done
    echo "feed ready at {{feed_url}}"
    cd "{{root}}/exercise"
    just run {{n}}

# boot the feed, run the Rust test suite against it, then tear the feed down
test:
    #!/usr/bin/env bash
    set -euo pipefail
    if curl -sf {{feed_url}}/symbols >/dev/null 2>&1; then
        echo "port 3000 already serving a feed; stop it first" >&2
        exit 1
    fi
    cd "{{root}}/services"
    cargo build
    ./target/debug/services --start-feed --num-accounts 20 --rate 2 &
    feed_pid=$!
    trap 'kill "$feed_pid" 2>/dev/null || true' EXIT INT TERM
    until curl -sf {{feed_url}}/symbols >/dev/null; do sleep 0.2; done
    echo "feed ready at {{feed_url}}"
    cd "{{root}}/exercise"
    just test

clean:
    cd "{{root}}/services" && cargo clean
    cd "{{root}}/exercise" && just clean
