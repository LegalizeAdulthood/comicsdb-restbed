#include <string>
#include <memory>
#include <string>
#include <cstdlib>
#include <ciso646>
#include <functional>
#include <restbed>

using namespace std;
using namespace restbed;

class HostRule : public Rule
{
    public:
        HostRule( void ) : Rule( )
        {
            return;
        }
        
        virtual ~HostRule( void )
        {
            return;
        }
        
        bool condition( const shared_ptr< Session > ) final override
        {
            return true;
        }
        
        void action( const shared_ptr< Session > session, const function< void ( const shared_ptr< Session > ) >& callback ) final override
        {
            const auto request = session->get_request( );
            
            if ( not request->has_header( "Host" ) )
            {
                session->close( BAD_REQUEST, "Bad Request! Host header required.", { { "Content-Length", "34" }, { "Content-Type", "text/plain" } } );
            }
            else
            {
                callback( session );
            }
        }
};

class AcceptRule : public Rule
{
    public:
        AcceptRule( void ) : Rule( )
        {
            return;
        }
        
        virtual ~AcceptRule( void )
        {
            return;
        }
        
        bool condition( const shared_ptr< Session > session ) final override
        {
            return session->get_request( )->has_header( "Accept" );
        }
        
        void action( const shared_ptr< Session > session, const function< void ( const shared_ptr< Session > ) >& callback ) final override
        {
            const auto request = session->get_request( );
            const auto type = request->get_header( "Accept", String::lowercase );
            
            if ( type not_eq "text/plain" and type not_eq "*/*" )
            {
                session->close( NOT_ACCEPTABLE,
                                "Not Acceptable, must be 'text/plain' or '*/*'.",
                { { "Content-Length", "46" }, { "Content-Type", "text/plain" } } );
            }
            else
            {
                callback( session );
            }
        }
};

void get_method_handler( const shared_ptr< Session > session )
{
    session->close( OK, "Hello, World!", { { "Content-Length", "13" }, { "Content-Type", "text/plain" } } );
}

int main( const int, const char** )
{
    auto resource = make_shared< Resource >( );
    resource->set_path( "/resource" );
    resource->add_rule( make_shared< AcceptRule >( ) );
    resource->set_method_handler( "GET", get_method_handler );
    
    auto settings = make_shared< Settings >( );
    settings->set_port( 1984 );
    settings->set_default_header( "Connection", "close" );
    
    Service service;
    service.publish( resource );
    service.add_rule( make_shared< HostRule >( ) );
    service.start( settings );
    
    return EXIT_SUCCESS;
}
