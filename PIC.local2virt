                Original message:
                   
                   To: dude@tommy.gov
                   Hi.
                   
qmail-inject    Fill in the complete envelope and header:
                   
     |             (envelope) from joe@heaven.af.mil to dude@tommy.gov
     |             From: joe@heaven.af.mil
     |             To: dude@tommy.gov
     |             
     |             Hi.
     V             
                   
qmail-queue     Store message safely on disk.
                Trigger qmail-send.
     |             
     V             
                   
qmail-send      Look at envelope recipient, dude@tommy.gov.
     |          Is tommy.gov in locals? No.
     |          Is dude@tommy.gov in virtualdomains? No.
     |          Is tommy.gov in virtualdomains? Yes: "tommy.gov:fred".
     |          Deliver locally to fred-dude@tommy.gov.
     V          
                   
qmail-lspawn ./Mailbox
                   
     |          Look at mailbox name, fred-dude.
     |          Is fred-dude listed in qmail-users? No.
     |          Is there a fred-dude account? No.
     |          Is there a fred account? Yes.
     |          Is fred's uid nonzero? Yes.
     |          Is ~fred visible to the qmailp user? Yes.
     |          Is ~fred owned by fred? Yes.
     |          Give control of the message to fred.
     |          Run qmail-local.
     V          
                   
qmail-local fred ~fred fred-dude - dude tommy.gov joe@heaven.af.mil ./Mailbox
                   
                Does ~fred/.qmail-dude exist? No.
                Does ~fred/.qmail-default exist? Yes: "./Mail.tommy".
                Write message to ./Mail.tommy in mbox format.
