#include <map>
#include <memory>
#include <cstdlib>
#include <ciso646>
#include <restbed>

#pragma GCC system_header
#pragma warning (disable: 4334)
#include "miniz.h" //https://github.com/richgel999/miniz
#pragma warning (default: 4334)

using namespace std;
using namespace restbed;

void deflate_method_handler( const shared_ptr< Session > session )
{
    const auto request = session->get_request( );

    int content_length = request->get_header( "Content-Length", 0 );

    session->fetch( content_length, [ request ]( const shared_ptr< Session > session, const Bytes & body )
    {
        Bytes result = body;

        if ( request->get_header( "Content-Encoding", String::lowercase ) == "deflate" )
        {
            mz_ulong length = compressBound( static_cast< mz_ulong >( body.size( ) ) );
            unique_ptr< unsigned char[ ] > data( new unsigned char[ length ] );
            const int status = uncompress( data.get( ), &length, body.data( ), static_cast< mz_ulong >( body.size( ) ) );

            if ( status not_eq MZ_OK )
            {
                const auto message = String::format( "Failed to deflate: %s\n", mz_error( status ) );
                session->close( 400, message, { { "Content-Length", ::to_string( message.length( ) ) }, { "Content-Type", "text/plain" } } );
                return;
            }

            result = Bytes( data.get( ), data.get( ) + length );
        }

        session->close( 200, result, { { "Content-Length", ::to_string( result.size( ) ) }, { "Content-Type", "text/plain" } } );
    } );
}

int main( const int, const char** )
{
    auto resource = make_shared< Resource >( );
    resource->set_path( "/api/deflate" );
    resource->set_method_handler( "POST", deflate_method_handler );

    auto settings = make_shared< Settings >( );
    settings->set_port( 1984 );
    settings->set_default_header( "Connection", "close" );

    Service service;
    service.publish( resource );
    service.start( settings );

    return EXIT_SUCCESS;
}
