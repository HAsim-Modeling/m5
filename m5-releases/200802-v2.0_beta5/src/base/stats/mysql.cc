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

#include <cassert>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "base/misc.hh"
#include "base/mysql.hh"
#include "base/statistics.hh"
#include "base/stats/flags.hh"
#include "base/stats/mysql.hh"
#include "base/stats/mysql_run.hh"
#include "base/stats/statdb.hh"
#include "base/stats/types.hh"
#include "base/str.hh"
#include "base/userinfo.hh"
#include "sim/host.hh"

using namespace std;

namespace Stats {

void
MySqlRun::connect(const string &host, const string &user, const string &passwd,
                  const string &db, const string &name, const string &sample,
                  const string &project)
{
    if (connected())
        panic("can only get one database connection at this time!");

    mysql.connect(host, user, passwd, db);
    if (mysql.error)
        panic("could not connect to database server\n%s\n", mysql.error);

    if (mysql.autocommit(false))
        panic("could not set autocommit\n%s\n", mysql.error);

    remove(name);
    //cleanup();
    setup(name, sample, user, project);
}

void
MySqlRun::setup(const string &name, const string &sample, const string &user,
                const string &project)
{
    assert(mysql.connected());

    stringstream insert;
    ccprintf(insert,
             "INSERT INTO "
             "runs(rn_name,rn_sample,rn_user,rn_project,rn_date,rn_expire)"
             "values(\"%s\", \"%s\", \"%s\", \"%s\", NOW(),"
             "DATE_ADD(CURDATE(), INTERVAL 31 DAY))",
             name, sample, user, project);

    mysql.query(insert);
    if (mysql.error)
        panic("could not get a run\n%s\n", mysql.error);

    run_id = mysql.insert_id();
    if (mysql.commit())
        panic("could not commit transaction\n%s\n", mysql.error);
}

void
MySqlRun::remove(const string &name)
{
    assert(mysql.connected());
    stringstream sql;
    ccprintf(sql, "DELETE FROM runs WHERE rn_name=\"%s\"", name);
    mysql.query(sql);
    if (mysql.error)
        panic("could not delete run\n%s\n", mysql.error);
    if (mysql.commit())
        panic("could not commit transaction\n%s\n", mysql.error);
}

void
MySqlRun::cleanup()
{
    assert(mysql.connected());

    mysql.query("DELETE data "
                "FROM data "
                "LEFT JOIN runs ON dt_run=rn_id "
                "WHERE rn_id IS NULL");

    if (mysql.commit())
        panic("could not commit transaction\n%s\n", mysql.error);

    mysql.query("DELETE formula_ref "
                "FROM formula_ref "
                "LEFT JOIN runs ON fr_run=rn_id "
                "WHERE rn_id IS NULL");

    if (mysql.commit())
        panic("could not commit transaction\n%s\n", mysql.error);

    mysql.query("DELETE formulas "
                "FROM formulas "
                "LEFT JOIN formula_ref ON fm_stat=fr_stat "
                "WHERE fr_stat IS NULL");

    if (mysql.commit())
        panic("could not commit transaction\n%s\n", mysql.error);

    mysql.query("DELETE stats "
                "FROM stats "
                "LEFT JOIN data ON st_id=dt_stat "
                "WHERE dt_stat IS NULL");

    if (mysql.commit())
        panic("could not commit transaction\n%s\n", mysql.error);

    mysql.query("DELETE subdata "
                "FROM subdata "
                "LEFT JOIN data ON sd_stat=dt_stat "
                "WHERE dt_stat IS NULL");

    if (mysql.commit())
        panic("could not commit transaction\n%s\n", mysql.error);

    mysql.query("DELETE events"
                "FROM events"
                "LEFT JOIN runs ON ev_run=rn_id"
                "WHERE rn_id IS NULL");

    if (mysql.commit())
        panic("could not commit transaction\n%s\n", mysql.error);

    mysql.query("DELETE event_names"
                "FROM event_names"
                "LEFT JOIN events ON en_id=ev_event"
                "WHERE ev_event IS NULL");

    if (mysql.commit())
        panic("could not commit transaction\n%s\n", mysql.error);
}

void
SetupStat::init()
{
    name = "";
    descr = "";
    type = "";
    print = false;
    prereq = 0;
    prec = -1;
    nozero = false;
    nonan = false;
    total = false;
    pdf = false;
    cdf = false;
    min = 0;
    max = 0;
    bktsize = 0;
    size = 0;
}

unsigned
SetupStat::setup(MySqlRun *run)
{
    MySQL::Connection &mysql = run->conn();

    stringstream insert;
    ccprintf(insert,
             "INSERT INTO "
             "stats(st_name, st_descr, st_type, st_print, st_prereq, "
             "st_prec, st_nozero, st_nonan, st_total, st_pdf, st_cdf, "
             "st_min, st_max, st_bktsize, st_size)"
             "values(\"%s\",\"%s\",\"%s\","
             "        %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)",
             name, descr, type, print, prereq, (int)prec, nozero, nonan,
             total, pdf, cdf,
             min, max, bktsize, size);

    mysql.query(insert);
    if (!mysql.error) {
        int id = mysql.insert_id();
        if (mysql.commit())
            panic("could not commit transaction\n%s\n", mysql.error);
        return id;
    }

    stringstream select;
    ccprintf(select, "SELECT * FROM stats WHERE st_name=\"%s\"", name);

    mysql.query(select);
    MySQL::Result result = mysql.store_result();
    if (!result)
        panic("could not find stat\n%s\n", mysql.error);

    assert(result.num_fields() == 16);
    MySQL::Row row = result.fetch_row();
    if (!row)
        panic("could not get stat row\n%s\n", mysql.error);

    bool tb;
    int8_t ti8;
    uint16_t tu16;
    int64_t ti64;
    uint64_t tu64;

    if (name != (char *)row[1])
        panic("failed stat check on %s:name. %s != %s\n",
              name, name, row[1]);

    if (descr != (char *)row[2])
        panic("failed stat check on %s:descr. %s != %s\n",
              name, descr, row[2]);

    if (type != (char *)row[3])
        panic("failed stat check on %s:type. %s != %s\n",
              name, type, row[3]);

    if (!to_number(row[4], tb) || print != tb)
        panic("failed stat check on %s:print. %d != %d\n",
              name, print, tb);

    if (!to_number(row[6], ti8) || prec != ti8)
        panic("failed stat check on %s:prec. %d != %d\n",
              name, prec, ti8);

    if (!to_number(row[7], tb) || nozero != tb)
        panic("failed stat check on %s:nozero. %d != %d\n",
              name, nozero, tb);

    if (!to_number(row[8], tb) || nonan != tb)
        panic("failed stat check on %s:nonan. %d != %d\n",
              name, nonan, tb);

    if (!to_number(row[9], tb) || total != tb)
        panic("failed stat check on %s:total. %d != %d\n",
              name, total, tb);

    if (!to_number(row[10], tb) || pdf != tb)
        panic("failed stat check on %s:pdf. %d != %d\n",
              name, pdf, tb);

    if (!to_number(row[11], tb) || cdf != tb)
        panic("failed stat check on %s:cdf. %d != %d\n",
              name, cdf, tb);

    if (!to_number(row[12], ti64) || min != ti64)
        panic("failed stat check on %s:min. %d != %d\n",
              name, min, ti64);

    if (!to_number(row[13], ti64) || max != ti64)
        panic("failed stat check on %s:max. %d != %d\n",
              name, max, ti64);

    if (!to_number(row[14], tu64) || bktsize != tu64)
        panic("failed stat check on %s:bktsize. %d != %d\n",
              name, bktsize, tu64);

    if (!to_number(row[15], tu16) || size != tu16)
        panic("failed stat check on %s:size. %d != %d\n",
              name, size, tu16);

    to_number(row[5], prereq);
    uint16_t statid;
    to_number(row[0], statid);
    return statid;
}

InsertData::InsertData(MySqlRun *_run)
    : run(_run)
{
    query = new char[maxsize + 1];
    size = 0;
    flush();
}

InsertData::~InsertData()
{
    delete [] query;
}

void
InsertData::flush()
{
    if (size) {
        MySQL::Connection &mysql = run->conn();
        assert(mysql.connected());
        mysql.query(query);
        if (mysql.error)
            panic("could not insert data\n%s\n", mysql.error);
        if (mysql.commit())
            panic("could not commit transaction\n%s\n", mysql.error);
    }

    query[0] = '\0';
    size = 0;
    first = true;
    strcpy(query, "INSERT INTO "
           "data(dt_stat,dt_x,dt_y,dt_run,dt_tick,dt_data) "
           "values");
    size = strlen(query);
}

void
InsertData::insert()
{
    if (size + 1024 > maxsize)
        flush();

    if (!first) {
        query[size++] = ',';
        query[size] = '\0';
    }

    first = false;

    size += sprintf(query + size, "(%u,%d,%d,%u,%llu,\"%f\")",
                    stat, x, y, run->run(), (unsigned long long)tick,
                    data);
}

InsertEvent::InsertEvent(MySqlRun *_run)
    : run(_run)
{
    query = new char[maxsize + 1];
    size = 0;
    first = true;
    flush();
}

InsertEvent::~InsertEvent()
{
    flush();
}

void
InsertEvent::insert(const string &stat)
{
    MySQL::Connection &mysql = run->conn();
    assert(mysql.connected());

    event_map_t::iterator i = events.find(stat);
    uint32_t event;
    if (i == events.end()) {
        mysql.query(
            csprintf("SELECT en_id "
                     "from event_names "
                     "where en_name=\"%s\"",
                     stat));

        MySQL::Result result = mysql.store_result();
        if (!result)
            panic("could not get a run\n%s\n", mysql.error);

        assert(result.num_fields() == 1);
        MySQL::Row row = result.fetch_row();
        if (row) {
            if (!to_number(row[0], event))
                panic("invalid event id: %s\n", row[0]);
        } else {
            mysql.query(
                csprintf("INSERT INTO "
                         "event_names(en_name)"
                         "values(\"%s\")",
                         stat));

            if (mysql.error)
                panic("could not get a run\n%s\n", mysql.error);

            event = mysql.insert_id();
        }
    } else {
        event = (*i).second;
    }

    if (size + 1024 > maxsize)
        flush();

    if (!first) {
        query[size++] = ',';
        query[size] = '\0';
    }

    first = false;

    size += sprintf(query + size, "(%u,%u,%llu)",
                    event, run->run(), (unsigned long long)curTick);
}

void
InsertEvent::flush()
{
    static const char query_header[] = "INSERT INTO "
        "events(ev_event, ev_run, ev_tick)"
        "values";

    MySQL::Connection &mysql = run->conn();
    assert(mysql.connected());

    if (size)
        mysql.query(query);

    query[0] = '\0';
    size = sizeof(query_header);
    first = true;
    memcpy(query, query_header, size);
}

struct InsertSubData
{
    uint16_t stat;
    int16_t x;
    int16_t y;
    string name;
    string descr;

    void setup(MySqlRun *run);
};

void
InsertSubData::setup(MySqlRun *run)
{
    MySQL::Connection &mysql = run->conn();
    assert(mysql.connected());
    stringstream insert;
    ccprintf(insert,
             "INSERT INTO subdata(sd_stat,sd_x,sd_y,sd_name,sd_descr) "
             "values(%d,%d,%d,\"%s\",\"%s\")",
             stat, x, y, name, descr);

    mysql.query(insert);
//    if (mysql.error)
//	panic("could not insert subdata\n%s\n", mysql.error);

    if (mysql.commit())
        panic("could not commit transaction\n%s\n", mysql.error);
}

MySql::MySql()
    : run(new MySqlRun), newdata(run), newevent(run)
{}

MySql::~MySql()
{
    delete run;
}

void
MySql::connect(const string &host, const string &user, const string &passwd,
               const string &db, const string &name, const string &sample,
               const string &project)
{
    run->connect(host, user, passwd, db, name, sample, project);
}

bool
MySql::connected() const
{
    return run->connected();
}

void
MySql::configure()
{
    /*
     * set up all stats!
     */
    using namespace Database;

    MySQL::Connection &mysql = run->conn();

    stat_list_t::const_iterator i, end = stats().end();
    for (i = stats().begin(); i != end; ++i) {
        (*i)->visit(*this);
    }

    for (i = stats().begin(); i != end; ++i) {
        StatData *data = *i;
        if (data->prereq) {
            // update the prerequisite
            uint16_t stat_id = find(data->id);
            uint16_t prereq_id = find(data->prereq->id);
            assert(stat_id && prereq_id);

            stringstream update;
            ccprintf(update, "UPDATE stats SET st_prereq=%d WHERE st_id=%d",
                     prereq_id, stat_id);
            mysql.query(update);
            if (mysql.error)
                panic("could not update prereq\n%s\n", mysql.error);

            if (mysql.commit())
                panic("could not commit transaction\n%s\n", mysql.error);
        }
    }

    if (mysql.commit())
        panic("could not commit transaction\n%s\n", mysql.error);

    configured = true;
}


bool
MySql::configure(const StatData &data, string type)
{
    stat.init();
    stat.name = data.name;
    stat.descr = data.desc;
    stat.type = type;
    stat.print = data.flags & print;
    stat.prec = data.precision;
    stat.nozero = data.flags & nozero;
    stat.nonan = data.flags & nonan;
    stat.total = data.flags & total;
    stat.pdf = data.flags & pdf;
    stat.cdf = data.flags & cdf;

    return stat.print;
}

void
MySql::configure(const ScalarData &data)
{
    if (!configure(data, "SCALAR"))
        return;

    insert(data.id, stat.setup(run));
}

void
MySql::configure(const VectorData &data)
{
    if (!configure(data, "VECTOR"))
        return;

    uint16_t statid = stat.setup(run);

    if (!data.subnames.empty()) {
        InsertSubData subdata;
        subdata.stat = statid;
        subdata.y = 0;
        for (int i = 0; i < data.subnames.size(); ++i) {
            subdata.x = i;
            subdata.name = data.subnames[i];
            subdata.descr = data.subdescs.empty() ? "" : data.subdescs[i];

            if (!subdata.name.empty() || !subdata.descr.empty())
                subdata.setup(run);
        }
    }

    insert(data.id, statid);
}

void
MySql::configure(const DistData &data)
{
    if (!configure(data, "DIST"))
        return;

    if (!data.data.fancy) {
        stat.size = data.data.size;
        stat.min = data.data.min;
        stat.max = data.data.max;
        stat.bktsize = data.data.bucket_size;
    }
    insert(data.id, stat.setup(run));
}

void
MySql::configure(const VectorDistData &data)
{
    if (!configure(data, "VECTORDIST"))
        return;

    if (!data.data[0].fancy) {
        stat.size = data.data[0].size;
        stat.min = data.data[0].min;
        stat.max = data.data[0].max;
        stat.bktsize = data.data[0].bucket_size;
    }

    uint16_t statid = stat.setup(run);

    if (!data.subnames.empty()) {
        InsertSubData subdata;
        subdata.stat = statid;
        subdata.y = 0;
        for (int i = 0; i < data.subnames.size(); ++i) {
            subdata.x = i;
            subdata.name = data.subnames[i];
            subdata.descr = data.subdescs.empty() ? "" : data.subdescs[i];
            if (!subdata.name.empty() || !subdata.descr.empty())
                subdata.setup(run);
        }
    }

    insert(data.id, statid);
}

void
MySql::configure(const Vector2dData &data)
{
    if (!configure(data, "VECTOR2D"))
        return;

    uint16_t statid = stat.setup(run);

    if (!data.subnames.empty()) {
        InsertSubData subdata;
        subdata.stat = statid;
        subdata.y = -1;
        for (int i = 0; i < data.subnames.size(); ++i) {
            subdata.x = i;
            subdata.name = data.subnames[i];
            subdata.descr = data.subdescs.empty() ? "" : data.subdescs[i];
            if (!subdata.name.empty() || !subdata.descr.empty())
                subdata.setup(run);
        }
    }

    if (!data.y_subnames.empty()) {
        InsertSubData subdata;
        subdata.stat = statid;
        subdata.x = -1;
        subdata.descr = "";
        for (int i = 0; i < data.y_subnames.size(); ++i) {
            subdata.y = i;
            subdata.name = data.y_subnames[i];
            if (!subdata.name.empty())
                subdata.setup(run);
        }
    }

    insert(data.id, statid);
}

void
MySql::configure(const FormulaData &data)
{
    MySQL::Connection &mysql = run->conn();
    assert(mysql.connected());

    configure(data, "FORMULA");
    insert(data.id, stat.setup(run));

    uint16_t stat = find(data.id);
    string formula = data.str();

    stringstream insert_formula;
    ccprintf(insert_formula,
             "INSERT INTO formulas(fm_stat,fm_formula) values(%d, \"%s\")",
             stat, formula);

    mysql.query(insert_formula);
//    if (mysql.error)
//	panic("could not insert formula\n%s\n", mysql.error);

    stringstream insert_ref;
    ccprintf(insert_ref,
             "INSERT INTO formula_ref(fr_stat,fr_run) values(%d, %d)",
             stat, run->run());

    mysql.query(insert_ref);
//    if (mysql.error)
//	panic("could not insert formula reference\n%s\n", mysql.error);

    if (mysql.commit())
        panic("could not commit transaction\n%s\n", mysql.error);
}

bool
MySql::valid() const
{
    return run->connected();
}

void
MySql::output()
{
    using namespace Database;
    assert(valid());

    if (!configured)
        configure();

    // store sample #
    newdata.tick = curTick;

    MySQL::Connection &mysql = run->conn();

    Database::stat_list_t::const_iterator i, end = Database::stats().end();
    for (i = Database::stats().begin(); i != end; ++i) {
        StatData *stat = *i;
        stat->visit(*this);
        if (mysql.commit())
            panic("could not commit transaction\n%s\n", mysql.error);
    }

    newdata.flush();
}

void
MySql::event(const std::string &event)
{
    newevent.insert(event);
}


void
MySql::output(const ScalarData &data)
{
    if (!(data.flags & print))
        return;

    newdata.stat = find(data.id);
    newdata.x = 0;
    newdata.y = 0;
    newdata.data = data.value();

    newdata.insert();
}

void
MySql::output(const VectorData &data)
{
    if (!(data.flags & print))
        return;

    newdata.stat = find(data.id);
    newdata.y = 0;

    const VCounter &cvec = data.value();
    int size = data.size();
    for (int x = 0; x < size; x++) {
        newdata.x = x;
        newdata.data = cvec[x];
        newdata.insert();
    }
}

void
MySql::output(const DistDataData &data)
{
    const int db_sum = -1;
    const int db_squares = -2;
    const int db_samples = -3;
    const int db_min_val = -4;
    const int db_max_val = -5;
    const int db_underflow = -6;
    const int db_overflow = -7;

    newdata.x = db_sum;
    newdata.data = data.sum;
    newdata.insert();

    newdata.x = db_squares;
    newdata.data = data.squares;
    newdata.insert();

    newdata.x = db_samples;
    newdata.data = data.samples;
    newdata.insert();

    if (data.samples && !data.fancy) {
        newdata.x = db_min_val;
        newdata.data = data.min_val;
        newdata.insert();

        newdata.x = db_max_val;
        newdata.data = data.max_val;
        newdata.insert();

        newdata.x = db_underflow;
        newdata.data = data.underflow;
        newdata.insert();

        newdata.x = db_overflow;
        newdata.data = data.overflow;
        newdata.insert();

        int size = data.cvec.size();
        for (int x = 0; x < size; x++) {
            newdata.x = x;
            newdata.data = data.cvec[x];
            newdata.insert();
        }
    }
}


void
MySql::output(const DistData &data)
{
    if (!(data.flags & print))
        return;

    newdata.stat = find(data.id);
    newdata.y = 0;
    output(data.data);
}

void
MySql::output(const VectorDistData &data)
{
    if (!(data.flags & print))
        return;

    newdata.stat = find(data.id);

    int size = data.data.size();
    for (int y = 0; y < size; ++y) {
        newdata.y = y;
        output(data.data[y]);
    }
}

void
MySql::output(const Vector2dData &data)
{
    if (!(data.flags & print))
        return;

    newdata.stat = find(data.id);

    int index = 0;
    for (int x = 0; x < data.x; x++) {
        newdata.x = x;
        for (int y = 0; y < data.y; y++) {
            newdata.y = y;
            newdata.data = data.cvec[index++];
            newdata.insert();
        }
    }
}

void
MySql::output(const FormulaData &data)
{
}

/*
 * Implement the visitor
 */
void
MySql::visit(const ScalarData &data)
{
    if (!configured)
        configure(data);
    else
        output(data);
}

void
MySql::visit(const VectorData &data)
{
    if (!configured)
        configure(data);
    else
        output(data);
}

void
MySql::visit(const DistData &data)
{
    return;
    if (!configured)
        configure(data);
    else
        output(data);
}

void
MySql::visit(const VectorDistData &data)
{
    return;
    if (!configured)
        configure(data);
    else
        output(data);
}

void
MySql::visit(const Vector2dData &data)
{
    return;
    if (!configured)
        configure(data);
    else
        output(data);
}

void
MySql::visit(const FormulaData &data)
{
    if (!configured)
        configure(data);
    else
        output(data);
}

bool
initMySQL(string host, string user, string password, string database,
          string project, string name, string sample)
{
    extern list<Output *> OutputList;
    static MySql mysql;

    if (mysql.connected())
        return false;

    mysql.connect(host, user, password, database, name, sample, project);
    OutputList.push_back(&mysql);

    return true;
}

/* end namespace Stats */ }