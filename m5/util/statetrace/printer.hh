#ifndef PRINTER_HH
#define PRINTER_HH

#include <iostream>
#include <string>
#include <vector>

#include "refcnt.hh"

class TraceChild;
class PrinterObject;

typedef RefCountingPtr<PrinterObject> PrinterPointer;

class PrinterObject : public RefCounted
{
  protected:
    TraceChild * child;
  public:
    PrinterObject(TraceChild * newChild) : child(newChild)
    {;}
    virtual std::ostream & writeOut(std::ostream & os) = 0;
    virtual bool configure(std::string) = 0;
};

class NestingPrinter : public PrinterObject
{
  private:
    std::vector<std::string> constStrings;
    std::vector<PrinterPointer> printers;
    int switchVar;
    int numPrinters;
  public:
    NestingPrinter(TraceChild * newChild) :
        PrinterObject(newChild), numPrinters(0), switchVar(-1)
    {;}

    bool configure(std::string);

    std::ostream & writeOut(std::ostream & os);
};

class RegPrinter : public PrinterObject
{
  private:
    int intRegNum;
  public:
    RegPrinter(TraceChild * newChild, int num = 0) :
        PrinterObject(newChild), intRegNum(num)
    {;}

    void regNum(int num)
    {
        intRegNum = num;
    }

    int regNum()
    {
        return intRegNum;
    }

    bool configure(std::string);

    std::ostream & writeOut(std::ostream & os);
};

static inline std::ostream & operator << (std::ostream & os,
        PrinterObject & printer)
{
    return printer.writeOut(os);
}

static inline std::ostream & operator << (std::ostream & os,
        PrinterPointer & printer)
{
    return printer->writeOut(os);
}

#endif
