#include <ctime>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <podofo/base/PdfData.h>
#include <podofo/base/PdfError.h>
#include <podofo/base/PdfArray.h>
#include <podofo/base/PdfFilter.h>
#include <podofo/base/PdfParser.h>
#include <podofo/base/PdfDictionary.h>
#include <podofo/base/PdfVecObjects.h>
#include <podofo/base/PdfFileStream.h>
#include <podofo/base/PdfOutputDevice.h>
#include <podofo/base/PdfOutputStream.h>

using namespace std;
using namespace PoDoFo;

/**
 * Skip some filters
 *
 * @param name - filter name
 * @return stream decode level
 */
bool getLevel(const string& name) {
    return !((name == "DCTDecode") || (name == "JPXDecode") || (name == "CCITTFaxDecode"));
}

/**
 * Print dictionary
 *
 * @param dictionary - dictionary object
 * @param old_level - old stream decode level
 * @return stream decode level
 */
bool printDictionary(PdfDictionary &dictionary, bool old_level) {
    cout << "<< ";
    bool level = old_level;
    for (auto &key : dictionary.GetKeys()) {
        cout << "/" << key.first.GetName();
        if (key.second->IsName()) {
            auto name = key.second->GetName().GetName();
            if (level)level = getLevel(name);
            cout << " /" << name << " ";
        }
        else if (key.second->IsNumber())
            cout << " " << key.second->GetNumber() << " ";
        else if (key.second->IsArray()) {
            auto data = key.second->GetArray();
            for (size_t i = 0; i < data.GetSize(); ++i) {
                cout << " ";
                auto item = data[i];
                if (item.IsDictionary()) {
                    auto new_level = printDictionary(item.GetDictionary(), level);
                    if (level)level = new_level;
                }
                else if (item.IsName()) {
                    const auto& name = item.GetName().GetName();
                    if (level)level = getLevel(name);
                    cout << name;
                }
                cout << " ";
            }
        }
    }
    cout << ">>";
    return level;
}

/**
 * PoDoFo library example
 *
 * Used PoDoFo library (http://podofo.sourceforge.net/) licensed under the Apache License, Version 2.0
 * Documentation: http://podofo.sourceforge.net/doc/html/index.html
 * Dependencies: apt-get install libpodofo libpodofo-dev [libpodofo-utils]
 */
int main(int argc, const char* argv[]) {
    // Start
    clock_t begin = clock();
    if(argc - 2) {
        cout << "Incorrect parameter(s)" << endl;
        return 1;
    }
    cout << "Parsing file '" << argv[1] << "':" << endl;
    int i=1;
    auto objects = PdfVecObjects();
    try {
        auto document = PdfParser(&objects);
        document.SetStrictParsing(false);
        document.SetIgnoreBrokenObjects(true);
        document.ParseFile(argv[1], true);
        string dir = string(argv[1]).append(".podofo_out");
        rmdir(dir.c_str());
        mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        for (auto object : objects) {
            if (object->HasStream()) {
                bool stream_decode = true;
                cout << "    Object " << i << " has stream ";
                auto dictionary = object->GetDictionary();
                stream_decode = printDictionary(dictionary, stream_decode);
                // Output file name
                ostringstream file_name;
                file_name << dir << "/pdf_";
                file_name.width(4);
                file_name.fill ('0');
                file_name << i << "_0.dat";
                auto stream = object->GetStream();
                auto output = PdfFileOutputStream(file_name.str().c_str());
                auto filters = PdfFilterFactory::CreateFilterList(object);
                try {
                    if (stream_decode)
                        stream->GetCopy(PdfFilterFactory::CreateDecodeStream(filters, &output, nullptr));
                    else {
                        stream->GetCopy(&output);
                        cout << " (filter is omitted)";
                    }
                }
                catch (PdfError &error) {
                    stream->GetCopy(&output);
                    cout << " (filter not supported)";
                }
                output.Close();
                cout << endl;
            }
            i++;
        }
        // Execution time output
        clock_t end = clock();
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        cout << endl << "Execution time: " << time_spent << " sec." << endl;
        return 0;
    }
    catch (PdfError& error) {
        cout << PoDoFo::PdfError::ErrorMessage(error.GetError()) << endl;
    }
}
