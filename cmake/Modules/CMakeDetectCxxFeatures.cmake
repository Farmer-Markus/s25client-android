# Checks for some C++ features and sets CXX_HAVE_{feature}
#
# Currently:
#     override

INCLUDE(CheckCXXSourceCompiles)

if(MSVC)
    # For MS we know this is available. Don't try to compile as it is slow
    SET(CXX_HAVE_OVERRIDE YES CACHE INTERNAL "")
else()
    UNSET(CXX_OVERRIDE_OK CACHE)
    CHECK_CXX_SOURCE_COMPILES("
        class Base{
        public:
            virtual int foo(int bar){ return bar; }
        };

        class Sub : public Base {
        public:
            virtual int foo(int bar) override{ return 2*bar; };
        };

        int main()
        {
            Base b;
            Sub s;

            return (b.foo(2) * 2 == s.foo(2)) ? 0 : 1;
        }
    " CXX_OVERRIDE_OK FAIL_REGEX "override controls .override/final. only available with"
    )

    # This should fail!
    UNSET(CXX_OVERRIDE_FAIL CACHE)
    CHECK_CXX_SOURCE_COMPILES("
        class Base{
        public:
            virtual int foo(int bar){ return bar; }
        };

        class Sub : public Base {
        public:
            virtual int foo2(int bar) override{ return 2*bar; };
        };

        int main()
        {
            Base b;
            Sub s;

            return (b.foo(2) * 2 == s.foo2(2)) ? 0 : 1;
        }
    " CXX_OVERRIDE_FAIL
    )
    
    IF(CXX_OVERRIDE_OK AND NOT CXX_OVERRIDE_FAIL)
        SET(CXX_HAVE_OVERRIDE YES CACHE INTERNAL "")
    ELSE()
        SET(CXX_HAVE_OVERRIDE NO CACHE INTERNAL "")
    ENDIF()
endif()

