# Direct ZeroMQ/Protobuf API

These examples keep the transport visible on purpose:

- request/reply via `GetProcessorSettings`
- pub/sub via `TargetData`

## Default Ports

See `proto/nav_api.proto` for the authoritative list and for detailed comments on each message and its conventions.

| Message | ZeroMQ Pattern | Default Port |
|---------|----------------|--------------|
| HydrophoneData | Publish-subscribe | 61501 |
| TargetData | Publish-subscribe | 61502 |
| ProcessorSettings | Publish-subscribe | 61503 |
| VesselInfo | Publish-subscribe | 61504 |
| GetProcessorSettingsRequest/Response | Request-reply | 60501 |
| SetFieldOfViewRequest/Response | Request-reply | 60502 |
| SetBottomDetectionRequest/Response | Request-reply | 60503 |
| SetInWaterSquelchRequest/Response | Request-reply | 60504 |
| SetSquelchlessInWaterDetectorRequest/Response | Request-reply | 60505 |
| GetVesselInfoRequest/Response | Request-reply | 60506 |

## Further Documentation

- **Proto files**: The `.proto` files in `proto/` contain inline comments that describe each message, field semantics, and conventions in more detail.
- **Interface Design Definition (IDD)**: [F33583-FarSounder_IDD_411.pdf](https://www.farsounder.com/s/F33583-FarSounder_IDD_411.pdf) — FarSounder's Interface Design Definition for Argos 350, 500, and 1000 systems, including coordinate system, hydrophone data, and target data.