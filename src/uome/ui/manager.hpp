#ifndef UOME_GUI_MANAGER_HPP
#define UOME_GUI_MANAGER_HPP

#include <ClanLib/core.h>
#include <ClanLib/application.h>
#include <ClanLib/display.h>
#include <ClanLib/gl.h>

#include <boost/shared_ptr.hpp>

namespace uome {
namespace ui {

class Manager {
public:
    static Manager* getSingleton();

    CL_DisplayWindow* getWindow();

    CL_GraphicContext& getGC();

    CL_InputContext& getIC();

    boost::shared_ptr<CL_Texture> provideTexture(unsigned int width, unsigned int height);

private:
    static Manager* singleton_;

    Manager();
    ~Manager();
    Manager(const Manager& copy) { }
    void operator=(const Manager& copy) { }

    CL_SetupCore clSetupCore_;
    CL_SetupDisplay clSetupDisplay_;
    CL_SetupGL clSetupGL_;

    CL_DisplayWindow* window_;
};

}
}

#endif
