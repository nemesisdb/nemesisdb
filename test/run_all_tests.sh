#!/bin/sh

# if [ -z "$1" ]
# then
#   /usr/bin/cmake --config Release --target clean
# fi

# /usr/bin/cmake --config Release -j 18

cd bin

echo
echo "Running all tests"
echo

for f in *; do
  ./"$f" || break  # execute successfully or break
  # Or more explicitly: if this execution fails, then stop the `for`:
  # if ! bash "$f"; then break; fi
done
