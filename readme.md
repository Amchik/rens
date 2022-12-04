<div align="center">
	<h1>rens 🗒</h1>
	<p>Local DNS server with DoH and caching</p>
</div>

## How it's works?

Just forward packages over HTTPS (DoH). Good if `systemd-resolved` is is not available
in your country.

Run `rens` as a service and set it as default nameserver. If you use `resolvconf`, do that:

```console
$ my-rc-services rens start-background
Started DNS server on 127.0.0.52:53
$ sudo su
# echo 'name_servers=127.0.0.52' >> /etc/resolvconf.conf
# resolvconf -u
```

Now you'r apps will use DoH for DNS queries.

TIP: if you use OpenRC as init system, you can use
[this service config](https://gist.github.com/ValgrindLLVM/fbc025d997f6499dcf2aaae63b8b36ee).

## Building

You need cURL (dev libs) and c compiler. On debian/ubuntu it can be downloaded
via `apt install libcurl4-openssl-dev build-essentials`.

Then, just type `make`:

```console
$ make
```

If you need custom configuration, copy `default-config.mk` to `config.mk` and edit it.

## Usage

```console
$ rens -h
Usage: bin/rens [path to config]
       bin/rens -h|--help
Default config path is /etc/rens.conf
```

Default configuration file located in `/etc/rens.conf`. If you want use default
params just type `touch /etc/rens.conf` as root.

### Example config

```
listen_ip localhost
listen_port 5353

dns_server 1.1.1.1
```

### As system DNS server (and resolver)

If you use openresolv, edit `/etc/resolvconf.conf` and add next line:

```
name_servers=127.0.0.52
```

Then type `sudo resolvconf -u`.

If you don't use resolvconf, just put `127.0.0.52` as you'r nameserver in
`/etc/resolv.conf`:

```console
$ echo 'nameserver 127.0.0.52' | sudo tee /etc/resolv.conf
```

`127.0.0.52` is a default IP address, you can use own in `/etc/rens.conf`.


