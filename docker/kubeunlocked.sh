#!/bin/bash
PASS="Welcome2KolabSystems"
kube --keyring '{"kolabnowAccount": {"kolabnowImap": "'"$PASS"'", "kolabnowSmtp": "'"$PASS"'", "kolabnowDav": "'"$PASS"'"}}'
