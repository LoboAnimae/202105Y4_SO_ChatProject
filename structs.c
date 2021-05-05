struct user
{
    char *name;
    int id;
    char *ip;
};

struct connected
{
    int id;
    char *status;
};

struct status
{
    int id;
    char *status;
};

// type = 0 -> GENERAL
// type = 1 -> DIRECT
struct comunication
{
    char *message;
    char *status;
    int id;
    char *name;
    int type;
};
