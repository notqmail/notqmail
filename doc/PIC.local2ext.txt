                Original message:
                   
                   To: fred-sos
                   Hi.
                   
qmail-inject    Fill in the complete envelope and header:
                   
     |             (envelope) from joe@heaven.af.mil to fred-sos@heaven.af.mil
     |             From: joe@heaven.af.mil
     |             To: fred-sos@heaven.af.mil
     |             
     |             Hi.
     V             
                   
qmail-queue     Store message safely on disk.
                Trigger qmail-send.
     |             
     V             
                   
qmail-send      Look at envelope recipient, fred-sos@heaven.af.mil.
     |          Is heaven.af.mil in locals? Yes.
     |          Deliver locally to fred-sos@heaven.af.mil.
     V          
                   
qmail-lspawn ./Mailbox
                   
     |          Look at mailbox name, fred-sos.
     |          Is fred-sos listed in qmail-users? No.
     |          Is there a fred-sos account? No.
     |          Is there a fred account? Yes.
     |          Is fred's uid nonzero? Yes.
     |          Is ~fred visible to the qmailp user? Yes.
     |          Is ~fred owned by fred? Yes.
     |          Give control of the message to fred.
     |          Run qmail-local.
     V          
                   
qmail-local fred ~fred fred-sos - sos heaven.af.mil joe@heaven.af.mil ./Mailbox
                   
                Does ~fred/.qmail-sos exist? Yes: "./Extramail".
                Write message to ./Extramail in mbox format.
