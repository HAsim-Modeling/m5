/*
 * Copyright (c) 2004, 2005
 * The Regents of The University of Michigan
 * All Rights Reserved
 *
 * This code is part of the M5 simulator.
 *
 * Permission is granted to use, copy, create derivative works and
 * redistribute this software and such derivative works for any
 * purpose, so long as the copyright notice above, this grant of
 * permission, and the disclaimer below appear in all copies made; and
 * so long as the name of The University of Michigan is not used in
 * any advertising or publicity pertaining to the use or distribution
 * of this software without specific, written prior authorization.
 *
 * THIS SOFTWARE IS PROVIDED AS IS, WITHOUT REPRESENTATION FROM THE
 * UNIVERSITY OF MICHIGAN AS TO ITS FITNESS FOR ANY PURPOSE, AND
 * WITHOUT WARRANTY BY THE UNIVERSITY OF MICHIGAN OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE. THE REGENTS OF THE UNIVERSITY OF MICHIGAN SHALL NOT BE
 * LIABLE FOR ANY DAMAGES, INCLUDING DIRECT, SPECIAL, INDIRECT,
 * INCIDENTAL, OR CONSEQUENTIAL DAMAGES, WITH RESPECT TO ANY CLAIM
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OF THE SOFTWARE, EVEN
 * IF IT HAS BEEN OR IS HEREAFTER ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGES.
 *
 * Authors: Nathan L. Binkert
 */

#ifndef __BASE_STATS_MYSQL_HH__
#define __BASE_STATS_MYSQL_HH__

#include <map>
#include <string>

#include "base/stats/output.hh"
#include "config/use_mysql.hh"

namespace MySQL { class Connection; }
namespace Stats {

class DistDataData;
class MySqlRun;

struct SetupStat
{
    std::string name;
    std::string descr;
    std::string type;
    bool print;
    uint16_t prereq;
    int8_t prec;
    bool nozero;
    bool nonan;
    bool total;
    bool pdf;
    bool cdf;
    double min;
    double max;
    double bktsize;
    uint16_t size;

    void init();
    unsigned setup(MySqlRun *run);
};

class InsertData
{
  private:
    char *query;
    int size;
    bool first;
    static const int maxsize = 1024*1024;

  public:
    MySqlRun *run;

  public:
    uint64_t tick;
    double data;
    uint16_t stat;
    int16_t x;
    int16_t y;

  public:
    InsertData(MySqlRun *_run);
    ~InsertData();

    void flush();
    void insert();
};

class InsertEvent
{
  private:
    char *query;
    int size;
    bool first;
    static const int maxsize = 1024*1024;

    typedef std::map<std::string, uint32_t> event_map_t;
    event_map_t events;

    MySqlRun *run;

  public:
    InsertEvent(MySqlRun *_run);
    ~InsertEvent();

    void flush();
    void insert(const std::string &stat);
};

class MySql : public Output
{
  protected:
    MySqlRun *run; /* Hide the implementation so we don't have a
                      #include mess */

    SetupStat stat;
    InsertData newdata;
    InsertEvent newevent;
    std::list<FormulaData *> formulas;
    bool configured;

  protected:
    std::map<int, int> idmap;

    void insert(int sim_id, int db_id)
    {
        using namespace std;
        idmap.insert(make_pair(sim_id, db_id));
    }

    int find(int sim_id)
    {
        using namespace std;
        map<int,int>::const_iterator i = idmap.find(sim_id);
        assert(i != idmap.end());
        return (*i).second;
    }

  public:
    MySql();
    ~MySql();

    void connect(const std::string &host, const std::string &user,
                 const std::string &passwd, const std::string &db,
                 const std::string &name, const std::string &sample,
                 const std::string &project);
    bool connected() const;

  public:
    // Implement Visit
    virtual void visit(const ScalarData &data);
    virtual void visit(const VectorData &data);
    virtual void visit(const DistData &data);
    virtual void visit(const VectorDistData &data);
    virtual void visit(const Vector2dData &data);
    virtual void visit(const FormulaData &data);

    // Implement Output
    virtual bool valid() const;
    virtual void output();

    // Implement Event Output
    virtual void event(const std::string &event);

  protected:
    // Output helper
    void output(const DistDataData &data);
    void output(const ScalarData &data);
    void output(const VectorData &data);
    void output(const DistData &data);
    void output(const VectorDistData &data);
    void output(const Vector2dData &data);
    void output(const FormulaData &data);

    void configure();
    bool configure(const StatData &data, std::string type);
    void configure(const ScalarData &data);
    void configure(const VectorData &data);
    void configure(const DistData &data);
    void configure(const VectorDistData &data);
    void configure(const Vector2dData &data);
    void configure(const FormulaData &data);
};

bool initMySQL(std::string host, std::string database, std::string user,
               std::string passwd, std::string project, std::string name,
               std::string sample);

#if !USE_MYSQL
inline bool
initMySQL(std::string host, std::string user, std::string password,
          std::string database, std::string project, std::string name,
          std::string sample)
{
    return false;
}
#endif

/* namespace Stats */ }

#endif // __BASE_STATS_MYSQL_HH__
