#ifndef LVPACKAGECONTEXT_H
#define LVPACKAGECONTEXT_H

#include "live/lvbaseglobal.h"
#include "live/package.h"
#include "live/plugin.h"

namespace lv{

class LV_BASE_EXPORT Package::Context{

public:
    Context(){}

    PackageGraph* packageGraph;
    std::list<Package::Ptr> dependents;
    std::list<Package::Ptr> dependencies;
    std::map<std::string, Plugin::Ptr> plugins;

private:
    // disable copy
    Context(const Package::Context&);
    void operator=(const Package::Context&);
};

} // namespace

#endif // LVPACKAGECONTEXT_H
