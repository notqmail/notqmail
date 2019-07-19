                Original message:
                   
                   To: help
                   Hi.
                   
qmail-inject    Fill in the complete envelope and header:
                   
     |             (envelope) from joe@heaven.af.mil to help@heaven.af.mil
     |             From: joe@heaven.af.mil
     |             To: help@heaven.af.mil
     |             
     |             Hi.
     V             
                   
qmail-queue     Store message safely on disk.
                Trigger qmail-send.
     |             
     V             
                   
qmail-send      Look at envelope recipient, help@heaven.af.mil.
     |          Is heaven.af.mil in locals? Yes.
     |          Deliver locally to help@heaven.af.mil.
     V          
                   
qmail-lspawn ./Mailbox
                   
     |          Look at mailbox name, help.
     |          Is help listed in qmail-users? No.
     |          Is there a help account? No.
     |          Give control of the message to alias.
     |          Run qmail-local.
     V          
                   
qmail-local alias ~alias help - help heaven.af.mil joe@heaven.af.mil ./Mailbox
                   
		Does ~alias/.qmail-help exist? Yes: "john".
		Forward message to john.
