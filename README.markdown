# lua-binding for libenigma

```lua
local enigma = require("enigma")
local keyf = io.open("dumpekey.bin", "rb")
local data = keyf:read("a")
print(data)
io.close(keyf)
assert(#data== 512)

local machine = enigma.new(data, function (x) if x%2==0 then return x+1 else return x-1 end end, function(x) return x end)
assert(machine:rollers()==2)
machine:roll(0,10)
machine:roll(1,20)
assert(machine:rollers()==2)
local p1, p2=machine:current_position()
assert(p1==10)
assert(p2==20)
assert(machine:test_replace())
assert(machine:test_reflect())

local encryptedf = io.open("encrypted.bin", "rb")
local encrypteddata = encryptedf:read("a")
io.close( encryptedf)
print(machine:encode(encrypteddata))
local newm = machine:dup()
assert(newm:test_replace())
assert(newm:test_reflect())
assert(newm:current_position()==machine:current_position())

print(machine:_ref())
print(newm:_ref())

```