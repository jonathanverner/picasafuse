SET(TINY_XML_SRC
  tinyxml.cpp
  tinyxmlparser.cpp
  tinyxmlerror.cpp
  tinystr.cpp
  ticpp.cpp  
)


#OBJS := $(addsuffix .o,$(basename ${SRCS}))


ADD_DEFINITIONS(
        -DDEBUG
        -g
        -DTIXML_USE_STL
        -DTIXML_USE_TICPP
)
