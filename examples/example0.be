#!/usr/bin/env berry

#-
 @filename  example0.be
 @version   1.0
 @autor     Máster Vitronic <mastervitronic@gmail.com>
 @date      Mon Mar 22 02:20:29 -04 2021
 @licence   MIT licence
-#

# Reads input file, applies vol & flanger effects, and play in soundcard.
# E.g. ./example0.be sound.ogg

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
	assert(false,'Failed to open input handler')
end

#Open soundcard
#Change "alsa" in this line to use an alternative audio device driver:
var out = sox.open_write('default',in, 'alsa')
if !out
	assert(false,'Failed to open out handler')
end


#Create an effects chain; some effects need to know about the input
#or output file encoding so we provide that information here
var chain = sox.create_effects_chain(in, out)
var effect = nil

#The first effect in the effect chain must be something that can source
#samples; in this case, we use the built-in handler that inputs
#data from an audio file
effect = sox.create_effect(sox.find_effect("input"))
	 sox.effect_options(effect, 1, in)
	 #This becomes the first `effect' in the chain
	 sox.add_effect(chain, effect, in, in)
effect = nil

#Create the `vol' effect, and initialise it with the desired parameters:
effect = sox.create_effect(sox.find_effect("vol"))
	 sox.effect_options(effect, 1, "3dB")
	 #Add the effect to the end of the effects processing chain:
	 sox.add_effect(chain, effect, in, in)
effect = nil

#Create the `flanger' effect, and initialise it with default parameters:
effect = sox.create_effect(sox.find_effect("flanger"))
	 sox.effect_options(effect, 0, nil)
	 #Add the effect to the end of the effects processing chain:
	 sox.add_effect(chain, effect, in, in)
effect = nil

#The last effect in the effect chain must be something that only consumes
#samples; in this case, we use the built-in handler that outputs
#data to an audio file
effect = sox.create_effect(sox.find_effect("output"))
	 sox.effect_options(effect, 1, out)
	 sox.add_effect(chain, effect, in, in)
effect = nil

#Flow samples through the effects processing chain until EOF is reached
sox.flow_effects(chain)

#All done; tidy up:
sox.close(in)
sox.close(out)
sox.quit()


