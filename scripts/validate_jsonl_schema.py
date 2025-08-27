#!/usr/bin/env python3
import sys, json
from jsonschema import validate, Draft202012Validator
schema = json.load(open('schemas/contact.schema.json'))
validator = Draft202012Validator(schema)
path = sys.argv[1] if len(sys.argv)>1 else 'examples/contacts.jsonl'
ok = True
with open(path) as f:
    for i, line in enumerate(f, 1):
        if not line.strip(): continue
        try:
            validate(json.loads(line), schema)
        except Exception as e:
            print(f"Line {i}: {e}")
            ok = False
print("Schema validation", "OK" if ok else "FAILED")
sys.exit(0 if ok else 1)
