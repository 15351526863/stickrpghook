#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include "..\SDK\Recv.h"
#include "..\SDK\IBaseClientDll.h"

#include <format>

namespace hash
{
    // fnv constants
    constexpr std::uint32_t BASE = 0x811C9DC5;
    constexpr std::uint32_t PRIME = 0x1000193;

    // compile-time FNV hash
    consteval std::uint32_t CompileTime( const char* data, const std::uint32_t value = BASE ) noexcept
    {
        return ( data[ 0 ] == '\0' ) ? value : CompileTime( &data[ 1 ], ( value ^ std::uint32_t( data[ 0 ] ) ) * PRIME );
    }

    // run-time FNV hash
    inline const std::uint32_t RunTime( const std::string_view data ) noexcept
    {
        std::uint32_t hashed = BASE;

        // hash characters
        for ( const char& character : data )
        {
            hashed ^= character;
            hashed *= PRIME;
        }

        return hashed;
    }
}

namespace netvar
{
    inline std::unordered_map<std::uint32_t, std::uint32_t> data = { };

    inline void Dump( const std::string_view base, RecvTable* table, const std::uint32_t offset = 0 )
    {
        for ( auto i = 0; i < table->GetNumProps( ); ++i ) {
            const auto prop = &table->pProps[ i ];

            if ( !prop )
                continue;

            if ( std::isdigit( prop->pVarName[ 0 ] ) )
                continue;

            if ( hash::RunTime( prop->pVarName ) == hash::CompileTime( "baseclass" ) )
                continue;

            // not a root table, dump again
            if ( prop->RecvType == SendPropType::DPT_DataTable &&
                prop->pDataTable &&
                prop->pDataTable->pNetTableName[ 0 ] == 'D' )
                Dump( base, prop->pDataTable, offset + prop->Offset );

            // place offset in netvar map
            data[ hash::RunTime( std::format( "{}->{}", base, prop->pVarName ).c_str( ) ) ] = offset + prop->Offset;
        }
    }

    inline void Setup( )
    {
        for ( auto client = g_pClientDll->GetAllClasses( ); client; client = client->pNext )
            if ( auto table = client->pRecvTable )
                Dump( client->pNetworkName, table, 0 );
    }
}

class NetvarTree 
{
    struct Node;
    using MapType = std::unordered_map<std::string, std::shared_ptr<Node>>;

    struct Node 
    {
        Node(int offset) : offset(offset) {}
        MapType nodes;
        int offset;
    };

    MapType nodes;

public:
    NetvarTree();

private:
    void PopulateNodes(class RecvTable *recv_table, MapType *map);

    /**
    * GetOffsetRecursive - Return the offset of the final node
    * @map:	Node map to scan
    * @acc:	Offset accumulator
    * @name:	Netvar name to search for
    *
    * Get the offset of the last netvar from map and return the sum of it and accum
    */
    static int GetOffsetRecursive(MapType &map, int acc, const char *name)
    {
        return acc + map[name]->offset;
    }

    /**
    * GetOffsetRecursive - Recursively grab an offset from the tree
    * @map:	Node map to scan
    * @acc:	Offset accumulator
    * @name:	Netvar name to search for
    * @args:	Remaining netvar names
    *
    * Perform tail recursion with the nodes of the specified branch of the tree passed for map
    * and the offset of that branch added to acc
    */
    template<typename ...args_t>
    int GetOffsetRecursive(MapType &map, int acc, const char *name, args_t ...args)
    {
        const auto &node = map[name];
        return this->GetOffsetRecursive(node->nodes, acc + node->offset, args...);
    }

public:
    /**
    * GetOffset - Get the offset of a netvar given a list of branch names
    * @name:	Top level datatable name
    * @args:	Remaining netvar names
    *
    * Initiate a recursive search down the branch corresponding to the specified datable name
    */
    template<typename ...args_t>
    int GetOffset(const char *name, args_t ...args)
    {
        const auto &node = nodes[name];
        return this->GetOffsetRecursive(node->nodes, node->offset, args...);
    }
};
extern std::unique_ptr<NetvarTree> g_pNetvars;