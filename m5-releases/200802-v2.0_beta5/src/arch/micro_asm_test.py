from micro_asm import MicroAssembler, Combinational_Macroop, Rom_Macroop, Rom

class Bah(object):
    def __init__(self):
        self.mnemonic = "bah"

class Bah_Tweaked(object):
    def __init__(self):
        self.mnemonic = "bah_tweaked"

class Hoop(object):
    def __init__(self, first_param, second_param):
        self.mnemonic = "hoop_%s_%s" % (first_param, second_param)
    def __str__(self):
        return "%s" % self.mnemonic

class Dah(object):
    def __init__(self):
        self.mnemonic = "dah"

microops = {
    "bah": Bah,
    "hoop": Hoop,
    "dah": Dah
}

class TestMacroop(Combinational_Macroop):
    def tweak(self):
        microops["bah"] = Bah_Tweaked
    def untweak(self):
        microops["bah"] = Bah
    def print_debug(self, message):
        print message

    def __init__(self, name):
        super(TestMacroop, self).__init__(name)
        self.directives = {
            "tweak": self.tweak,
            "untweak": self.untweak,
            "print": self.print_debug
        }

assembler = MicroAssembler(TestMacroop, microops, Rom('main ROM'), Rom_Macroop)

testAssembly = '''
# Single line comment

def rom {
    goo: bah
    extern la: hoop 4*8, "a"
}; /* multiline comment on one line */

/* multi line comment across lines
   to make sure they work */

def macroop squishy {
    .tweak
    bah
    .untweak
    .print "In the midst"
    bah
    dah # single line comment after something
    .tweak
};

#Extending the rom...
def rom
{
    #Here's more stuff for the rom
    bah
};

def macroop squashy {
    bah
};

def macroop jumper (bar);
'''
assembler.assemble(testAssembly)
