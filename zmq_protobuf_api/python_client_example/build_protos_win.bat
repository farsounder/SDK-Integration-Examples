echo off
pushd %~dp0
set ROOT=%~dp0
set COMPILED_PROTO_DIR=%ROOT%python_proto
set PROTO_FILES=%ROOT%proto_files
set PROTOBUF_BIN=%ROOT%protobuf_bin
set PROTOC=%PROTOBUF_BIN%\protoc.exe

md %COMPILED_PROTO_DIR%

REM Change into proto-files directory, loop over them
REM and compile them into COMPILED_PROTO_DIR
pushd %PROTO_FILES%
for %%F in (*.proto) do (
  echo Compiling %%F into python files
  %PROTOC% --proto_path %PROTO_FILES% --python_out %COMPILED_PROTO_DIR% %%F
)
popd
