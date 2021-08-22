qmail-smtpd     Receive message by SMTP from another host:
                   
                   MAIL FROM:<spammer@aol.com>
                   RCPT TO:<bill@irs.gov>
                
                Is $RELAYCLIENT set? No.
                Is irs.gov in rcpthosts? No.
                Reject RCPT.
