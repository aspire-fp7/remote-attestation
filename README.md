# Remote Attestation infrastructure for ASPIRE

This repository contains both client code and server code.

This code is supposed to work run in the ASPIRE framework, see [the main organization on Github](https://github.com/aspire-fp7). In that context, it can be automatically
combined with code mobility, so that clients with failed attestations (or without attestations) do not receive mobile code blocks from
the server.

This code makes use of OpenSSL, MySQL, and blake2 (the latter through files in src/headers/external/ and src/ra-additionals/external/)

# Publications making use of this code
* [Reactive Attestation: Automatic Detection and Reaction to Software Tampering Attacks](https://www.sigsac.org/ccs/CCS2016/wp-content/uploads/2016/08/Open-TOC-SPRO.html).
  Alessio Viticchié, Cataldo Basile, Andrea Avancini, Mariano Ceccato, Bert Abrath, Bart Coppens.
  In SPRO '16 - Proceedings of the 2016 ACM Workshop on Software PROtection

