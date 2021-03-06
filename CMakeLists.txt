cmake_minimum_required( VERSION 3.0.0 )
project( glAArgDemo )

# find a supported inline variant
include( CheckCSourceCompiles )
foreach( KEYWORD "inline" "__inline__" "__inline" )
    unset( inline CACHE )
    check_c_source_compiles(
        "
        typedef int foo_t;
        static ${KEYWORD} foo_t static_foo(){return 0;}
        foo_t foo(){return 0;}
        int main(int argc, char *argv[]){return 0;}
        "
        inline
    )
    if( inline )
        add_definitions( "-Dinline=${KEYWORD}" )
        break()
    endif( inline )
endforeach( KEYWORD )

# needed for ntohl()
find_library( WS2_32_LIBRARY ws2_32 )

# give FindGLUT.cmake a helping hand
if( WIN32 )
    set( GLUT_ROOT_PATH "c:/projects/root" )
endif( WIN32 )

# find & include OpenGL
find_package ( OpenGL REQUIRED )
if( NOT OPENGL_FOUND )
    message( ERROR " OpenGL not found!" )
endif( NOT OPENGL_FOUND )
include_directories( ${OPENGL_INCLUDE_DIR} )

# find & include GLUT
find_package( GLUT REQUIRED )
if( NOT GLUT_FOUND )
    message( ERROR " GLUT not found!" )
endif( NOT GLUT_FOUND )
include_directories( ${GLUT_INCLUDE_DIR} )
if( WIN32 )
    # we're using find_package() so we don't need FreeGLUT being helpful
    add_definitions( -DFREEGLUT_LIB_PRAGMAS=0 )
endif( WIN32 )

#find_package ( GLEW REQUIRED STATIC )
#target_link_libraries( exename ${GLEW_LIBRARY}

if( MSVC )
    add_definitions( /D _CRT_SECURE_NO_WARNINGS )
endif( MSVC )

include_directories( include )

add_executable( glAArgDemo "src/main.c" "src/AAPrimitives.c" )
target_link_libraries( glAArgDemo ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} )
if( WIN32 )
  target_link_libraries( glAArgDemo ws2_32 )
endif( WIN32 )