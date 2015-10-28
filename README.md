This Might Be Offensive
=======================

Getting Started
---------------

1. Install vagrant and virtualbox. If you're on a Mac: `brew install Caskroom/cask/vagrant Caskroom/cask/virtualbox`
2. `git clone git@github.com:numist/this-might-be-offensive.git`
3. `cd this-might-be-offensive`
4. `vagrant up`
5. Point your browser to [`https://localhost:8080/offensive`](https://localhost:8080/offensive/)
6. Log in as either `admin`/`[nsfw]` or `asdf`/`[tmbo]`.

The codebase is installed in the vm at `~/sites/tmbo`

### Common Gotchas ###

#### Realtime Data ####

Self-signed certs can pose a problem for realtime, but launching Chrome with `--ignore-certificate-errors` will get it working.

#### Hotlinking Restrictions ####

tmbo's content protection is the same in development as it is in production, which means if you browse to your instance by IP, you're not going to see any images. You should either disable the relevant section in the nginx config, or add the ip address you use to the allowed hosts. 

Help!
-----

Problems with the web site are frequently well-documented by error messages emitted by trigger-error. Administrators see this output as part of the rendered page, but it is also recorded to the httpd's logs. On the VM they are located in `~/logs/`.

If you need anything to get running, help can usually be had in #tmbotech on EFnet.
