#include <Xm/Picture.h>

#define NUMDIGIT      '#'
#define HEXDIGIT      'x'
#define OCTALDIGIT    'o'
#define NONCASELETTER '?'
#define UPCASELETTER  '&'
#define NONCASECHAR   '@'
#define UPCASECHAR    '!'
#define ESCAPE        ';'
#define CLOSURE       '*'
#define LBRACKET      '['
#define RBRACKET      ']'
#define LBRACE        '{'
#define RBRACE        '}'
#define ALTERNATIVE   ','

#define NODE_START_COUNT 40

typedef enum {
    NullTransition,		/* Transition on no input */
    NumericDigit,		/* eqivalent to [0-9] */
    HexDigit,			/* eqivalent to [0-9a-fA-F] */
    OctalDigit,			/* eqivalent to [0-7] */
    AnyLetter,			/* [a-zA-Z] */
    UpCaseLetter,		/* ditto, but translates [a-zA-Z] -> [A-Z] */
    AnyCharacter,		/* Any printing character */
    UpCaseCharacter,		/* ditto, case transition as above */
    LiteralCharacter		/* Single character */
} XmTransType;

typedef struct _XmPictureTransition {
    int                 destination;   /* Node to transition to */
    XmTransType         type;	       /* literal, null, upcasechar, etc... */
    char                c;	       /* key -- used for literals */
				       /* OR: count for closures */
    struct _XmPictureTransition *next; /* Next transition from our node */
} XmPictureTransition;

typedef struct _XmPictureNode {
    int                  index;
    XmPictureTransition *transitions;
} XmPictureNode;

typedef struct _XmPictureRec {
    char          * source;	/* string it was parsed from */
    int             num_nodes;
    int             nodes_alloced;
    int             start_node;
    int             final_node;
    XmPictureNode **nodes;	/* array of nodes */
} XmPictureRec;

typedef struct _XmPictureStateRec {
    XmPictureRec  *picture;
    char          *current_string;
    char          *append;
    int            statesize;
    unsigned char *state;	/* bitvector of states */
    unsigned char *newstate;	/* scratch space for use in
				   transitions */
    char           current;	/* currently added character */
    Boolean        upcase;
} XmPictureStateRec;

typedef struct _XmAutoFill {
    char    c;			/* char to fill */
    Boolean reject;		/* literal's didn't match: it's "right out" */
    Boolean digit;		/* isdigit(c) must be true */
    Boolean upcase;		/* isupper(c) must be true */
    Boolean letter;		/* isalpha(c) must be true */
    Boolean hexdigit;		/* isxdigit(c) must be true */
    Boolean octaldigit;		/* must be 0-7 */
} XmAutoFill;
