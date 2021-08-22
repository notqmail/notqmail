                Original message:
                   
                   To: fred
                   Hi.
                   
qmail-inject    Fill in the complete envelope and header:
                   
     |             (envelope) from joe@heaven.af.mil to fred@heaven.af.mil
     |             From: joe@heaven.af.mil
     |             To: fred@heaven.af.mil
     |             
     |             Hi.
     V             
                   
qmail-queue     Store message safely on disk.
                Trigger qmail-send.
     |             
     V             
                   
qmail-send      Look at envelope recipient, fred@heaven.af.mil.
     |          Is heaven.af.mil in locals? Yes.
     |          Deliver locally to fred@heaven.af.mil.
     V          
                   
qmail-lspawn ./Mailbox
                   
     |          Look at mailbox name, fred.
     |          Is fred listed in qmail-users? No.
     |          Is there a fred account? Yes.
     |          Is fred's uid nonzero? Yes.
     |          Is ~fred visible to the qmailp user? Yes.
     |          Is ~fred owned by fred? Yes.
     |          Give control of the message to fred.
     |          Run qmail-local.
     V          
                   
qmail-local fred ~fred fred '' '' heaven.af.mil joe@heaven.af.mil ./Mailbox
                   
                Does ~fred/.qmail exist? No.
                Write message to ./Mailbox in mbox format.
