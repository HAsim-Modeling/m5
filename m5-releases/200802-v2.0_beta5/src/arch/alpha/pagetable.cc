
#include "arch/alpha/pagetable.hh"
#include "sim/serialize.hh"

namespace AlphaISA
{
    void
    TlbEntry::serialize(std::ostream &os)
    {
        SERIALIZE_SCALAR(tag);
        SERIALIZE_SCALAR(ppn);
        SERIALIZE_SCALAR(xre);
        SERIALIZE_SCALAR(xwe);
        SERIALIZE_SCALAR(asn);
        SERIALIZE_SCALAR(asma);
        SERIALIZE_SCALAR(fonr);
        SERIALIZE_SCALAR(fonw);
        SERIALIZE_SCALAR(valid);
    }

    void
    TlbEntry::unserialize(Checkpoint *cp, const std::string &section)
    {
        UNSERIALIZE_SCALAR(tag);
        UNSERIALIZE_SCALAR(ppn);
        UNSERIALIZE_SCALAR(xre);
        UNSERIALIZE_SCALAR(xwe);
        UNSERIALIZE_SCALAR(asn);
        UNSERIALIZE_SCALAR(asma);
        UNSERIALIZE_SCALAR(fonr);
        UNSERIALIZE_SCALAR(fonw);
        UNSERIALIZE_SCALAR(valid);
    }
}
