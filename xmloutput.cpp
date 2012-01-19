#include "global.h"
#include "Function.h"
#include "ticpp.h"

#include <sys/stat.h>
#include <map>
#include <fstream>

using namespace std;

void Q2XMLReport(const string& q2xmlfilename, const string& q2app, const map<string, Function*>& functions)
{
    
    if(access(q2xmlfilename.c_str(),F_OK) != 0)
    {
        ofstream xmlfile;
        xmlfile.open(q2xmlfilename);
        // initialize file
        xmlfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
        xmlfile << "<q2:profiles xmlns:q2=\"http://www.example.org/q2profiling\"" << endl;
        xmlfile << "             xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << endl;
        xmlfile << "             xsi:schemaLocation=\"http://www.example.org/q2profiling q2profiling.xsd \">" << endl;
        xmlfile << "</q2:profiles>" << endl;
        xmlfile.close();
    }

    try{
        ticpp::Document q2xml(q2xmlfilename);
        ticpp::Element *app;

        q2xml.LoadFile();
        // find application with name q2app
        ticpp::Iterator< ticpp::Element > it(g_namespace + "application");
        for(it = it.begin(q2xml.FirstChildElement(g_namespace + "profiles"));
                it != it.end();
                it++)
            if(it->GetAttribute("name").compare(q2app) == 0)
                break;
        if(it != it.end())
            app = it.Get();
        else {
            app = new ticpp::Element(g_namespace + "application");
            app->SetAttribute("name",q2app);
            app->LinkEndChild(new ticpp::Element(g_namespace + "functions"));
            app->LinkEndChild(new ticpp::Element(g_namespace + "QDUGraph"));
            app->LinkEndChild(new ticpp::Element(g_namespace + "CallGraph"));
            q2xml.FirstChildElement()->LinkEndChild(app);
        }

    cout << __LINE__ << endl;
        //visit all functions and put them in a named map
        map<string, ticpp::Element*> function_tags;
        //for each fn in app
        it = ticpp::Iterator< ticpp::Element >(g_namespace + "function");
        for(it = it.begin(app->FirstChildElement(g_namespace + "functions"));
                it != it.end();
                it++)
            function_tags[it->GetAttribute("name")] = it.Get();

        //print functions
        map<string, Function*>::const_iterator mit;
        for(mit = functions.begin();
                mit != functions.end();
                mit++)
        {
            const string& fnname = mit->first;
            Function *function = mit->second;
            ticpp::Element *tag = function_tags[fnname];

            if(!tag) {
                tag = new ticpp::Element(g_namespace + "function");
                tag->SetAttribute("name",fnname);
                tag->LinkEndChild(new ticpp::Element(g_namespace + "metrics"));
                tag->LinkEndChild(new ticpp::Element(g_namespace + "statistics"));
                app->FirstChildElement(g_namespace + "functions")->LinkEndChild(tag);
            }
            function->outputXML(tag);
        }

        q2xml.SaveFile();
    } catch( ticpp::Exception& ex) {
        cout << "Error processing Q2 XML..." << endl;
        cout << ex.what() << endl;
        exit(0);
    }
}
