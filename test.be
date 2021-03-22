#!/usr/bin/env berry

#-
 @filename  test.be
 @version   1.0
 @autor     Máster Vitronic <mastervitronic@gmail.com>
 @date      Mon Mar 22 00:48:10 -04 2021
 @licence   MIT licence
-#

import libsox as sox
import string


if _argv.size() <= 1 
	print(string.format('Usage: %s sound.[ogg|mp3|wav|opus]', _argv[0]))
	return false
end

#All libSoX applications must start by initialising the SoX library
if !sox.init()
	assert(false,'Failed to initialize SoX')
end

#Open audiofile
var in  = sox.open_read(_argv[1])
if !in
	assert(false,'Failed to open file')
end

#Open soundcard
#Change "alsa" in this line to use an alternative audio device driver:
var out = sox.open_write('default',in, 'alsa')
#Or if you prefer, store all this to another file
#var out = sox.open_write('/tmp/coso.mp3',in, 'mp3')
if !out
	assert(false,'Failed to open out')
end


buf_sz = 8192*2
buffer = sox.buffer(buf_sz)
while true
	var sz = sox.read(in,buffer,buf_sz)
	if sz == 0 break end
	sox.write(out, buffer, sz)
end


sox.free_buffer(buffer)
sox.close(in)
sox.close(out)
sox.quit()
