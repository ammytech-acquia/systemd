# Acquia systemd

This is Acquia's copy of <https://code.launchpad.net/~usd-import-team/ubuntu/+source/systemd/+git/systemd>
which itself imports from Upstream <https://github.com/systemd/systemd>.

This repo contains backports from systemd upstream for use in Ubuntu.

## Branches

[ubuntu/xenial-updates-acquia](https://github.com/acquia/systemd/tree/ubuntu/xenial-updates-acquia): Patches added on top of xenial-updates.

## Updating branches

1.  Checkout an acquia suffix branch.
2.  Rebase on the updated branch: ubuntu/{OSNAME}-{CHANNEL}, e.g. ubuntu/xenial-updates.
3.  If needed, remove acquia patches integrated upstream.
4.  Fix changelog message and version.
5.  Push to GitHub and Launchpad.
6.  Request a build from Launchpad for the correct version: e.g. <https://code.launchpad.net/~acquia/+recipe/systemd-xenial>
