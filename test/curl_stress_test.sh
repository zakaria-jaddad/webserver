#!/bin/bash

for i in {1..1000}; do
  echo "Request $i"
  curl http://localhost:2010
  echo ""
done
