enum
{
    jiLine = 0x01,
    jiVertex = 0x02
};

typedef struct
{
    bool fJoinType; // joined to a line, or a vertex?
    union
    {
    int iVertex;    // joined to a vertex? - vertex number
    int iLine;      // joined to a line.
    };
    int x, y;       // x and y positions of join point

} JoinInfo;

bool CheckJoin( int iVertex, JoinInfo& ji, int iDistance = 32000,
        unsigned uFlags = jiLine | jiVertex );
