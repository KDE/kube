#!/bin/bash
PASS="Welcome2KolabSystems"
PASS2="doe"
kube --keyring '{"kolabnowAccount": {"kolabnowImap": "'"$PASS"'", "kolabnowSmtp": "'"$PASS"'", "kolabnowCarddav": "'"$PASS"'", "kolabnowCaldav": "'"$PASS"'"}, "perfAccount": {"perfImap": "'"$PASS2"'", "perfSmtp": "'"$PASS2"'", "perfCarddav": "'"$PASS2"'", "perfCaldav": "'"$PASS2"'"}}'
