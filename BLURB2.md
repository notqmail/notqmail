Mailing list management is one of qmail's strengths. Notable features:

* qmail lets each user handle his own mailing lists. The delivery
instructions for user-whatever go into ~user/.qmail-whatever.

* qmail makes it really easy to set up mailing list owners. If the user
touches ~user/.qmail-whatever-owner, all bounces will come back to him.

* qmail supports VERPs, which permit completely reliable automated
bounce handling for mailing lists of any size.

* SPEED---qmail blasts through mailing lists an order of magnitude
faster than sendmail. For example, one message was successfully
delivered to 150 hosts around the world in just 70 seconds, with qmail's
out-of-the-box configuration.

* qmail automatically prevents mailing list loops, even across hosts.

* qmail allows inconceivably gigantic mailing lists. No random limits.

* qmail handles aliasing and forwarding with the same simple mechanism.
For example, Postmaster is controlled by ~alias/.qmail-postmaster. This
means that cross-host loop detection also applies to aliases.

* qmail supports the ezmlm mailing list manager, which easily and
automatically handles bounces, subscription requests, and archives.
