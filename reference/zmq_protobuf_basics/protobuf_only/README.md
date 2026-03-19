# Protobuf Only

This example checks protobuf code generation and basic serialize/deserialize
logic without involving ZeroMQ or SonaSoft.

## Build

```bash
cmake -S . -B build
cmake --build build --config Release
```

## Run

Write an address book file:

```bash
./build/Release/write_addresses.exe address_book.bin # windows
./build/write_addresses address_book.bin # linux
```

Then read it back:

```bash
./build/Release/read_addresses.exe address_book.bin # windows
./build/read_addresses address_book.bin # linux
```

This example is from the protobuf docs: https://protobuf.dev/getting-started/cpptutorial/