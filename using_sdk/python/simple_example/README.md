# Python SDK Simple Example

This is the recommended Python integration path. It uses the higher-level
`farsounder` package instead of managing ZeroMQ and protobuf directly.

This folder is already set up as a `uv` project with `farsounder` added as a
dependency.

## Run

```bash
uv run main.py
```

To run without `uv`, ensure `farsounder` and its dependencies are installed (e.g., with `pip install farsounder`), then use:

```bash
python main.py
```

## What It Demonstrates

- building a client config
- making a request/reply call with `get_processor_settings`
- subscribing to `TargetData`
- stopping the subscriber cleanly

The canonical SDK repo is
[farsounder-python-client](https://github.com/farsounder/farsounder-python-client).
