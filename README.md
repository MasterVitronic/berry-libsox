## berry-libsox: Berry bindings for libSoX

berry-libsox is a Berry binding library for the [Swiss Army knife of sound processing programs (SoX)](http://sox.sourceforge.net/).

It runs on GNU/Linux and requires [Berry](https://github.com/Skiars/berry/) (>=0.1.10)
and [SoX](http://sox.sourceforge.net/) (>=14.4.2).

_Authored by:_ _[Díaz Devera Víctor Diex Gamar (Máster Vitronic)](https://www.linkedin.com/in/Master-Vitronic)_

[![Berry logo](./docs/berry.svg)](https://github.com/Skiars/berry/)

#### License

MIT license . See [LICENSE](./LICENSE).

#### Documentation

See the [Reference Manual](https://vitronic.gitlab.io/berry-libsox/).

#### Motivation:

This is a port of [lua-libsox](https://gitlab.com/vitronic/lua-libsox)
to test the capabilities of [berry](https://github.com/Skiars/berry/) as a
general purpose programming language.


#### Getting and installing

```sh
$ git clone https://gitlab.com/vitronic/berry-libsox.git
$ cd berry-libsox
berry-libsox$ make
berry-libsox$ make install # or 'sudo make install' (Ubuntu)
```

#### Example

```ruby
#- Script: player.be

import libsox as sox

assert(sox.init())

input  = sox.open_read(_argv[1])
output = sox.open_write('default',input, 'alsa')

buf_sz = 8192*2
buffer = sox.buffer(buf_sz)
while true
	var sz = sox.read(input,buffer,buf_sz)
	if sz == 0 break end
	sox.write(output, buffer, sz)
end


sox.free_buffer(buffer)
sox.close(input)
sox.close(output)
sox.quit()
```

The script can be executed at the shell prompt with the standard Lua interpreter:

```shell
$ berry player.be sound.ogg
```

Other examples can be found in the **examples/** directory contained in the release package

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.
