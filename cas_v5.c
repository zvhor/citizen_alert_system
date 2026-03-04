#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_STR 64
#define MAX_MSG 256

#define ROLE_OFFICE  0
#define ROLE_CITIZEN 1

#define OFFICE_ROLE_SUPER  0
#define OFFICE_ROLE_ADMIN  1

#define PRIO_LOW       0
#define PRIO_NORMAL    1
#define PRIO_HIGH      2
#define PRIO_EMERGENCY 3

#define TGT_ALL       0
#define TGT_REGION    1
#define TGT_STATUS    2
#define TGT_EDUCATION 3


typedef struct OfficeAccount {
    int  id;
    char username[MAX_STR];
    char password[MAX_STR];
    char office_name[MAX_STR];
    int  role;        
    int  active;
    struct OfficeAccount *next;
} OfficeAccount;

typedef struct Citizen {
    int  id;
    char name[MAX_STR];
    char region[MAX_STR];
    int  status;
    int  education;
    struct Citizen *next;
} Citizen;

typedef struct NotifNode {
    int  id;
    char title[MAX_STR];
    char message[MAX_MSG];
    char office[MAX_STR];
    int  priority;
    int  target_type;
    char target_value[MAX_STR];
    char created_at[MAX_STR];
    int  active;
    struct NotifNode *next;
} NotifNode;

typedef struct {
    NotifNode *front;
    NotifNode *rear;
    int size;
} Queue;



static OfficeAccount *office_list  = NULL;
static Citizen       *citizen_list = NULL;
static Queue          notif_queue  = {NULL, NULL, 0};

static int oid_counter = 1;
static int cid_counter = 100;
static int nid_counter = 1;


static const char *prio_str(int p) {
    const char *t[] = {"Low","Normal","High","EMERGENCY"};
    return (p>=0&&p<=3) ? t[p] : "?";
}
static const char *tgt_str(int t) {
    const char *s[] = {"All","Region","Status","Education"};
    return (t>=0&&t<=3) ? s[t] : "?";
}
static const char *status_str(int s) {
    const char *t[] = {"Student","Worker","General"};
    return (s>=0&&s<=2) ? t[s] : "?";
}
static const char *edu_str(int e) {
    const char *t[] = {"Primary","O-Level","A-Level","Diploma","Undergrad","Postgrad"};
    return (e>=0&&e<=5) ? t[e] : "?";
}
static const char *office_role_str(int r) {
    return r == OFFICE_ROLE_SUPER ? "Super Admin" : "Admin";
}

static void clear_stdin(void) {
    int c; while ((c=getchar())!='\n'&&c!=EOF);
}

static void get_time(char *buf, int len) {
    time_t t = time(NULL);
    strftime(buf, len, "%Y-%m-%d %H:%M", localtime(&t));
}


static void enqueue(NotifNode n) {
    NotifNode *node = malloc(sizeof(NotifNode));
    if (!node) { printf("Memory error!\n"); return; }
    *node = n;
    node->next = NULL;
    if (notif_queue.rear == NULL)
        notif_queue.front = notif_queue.rear = node;
    else {
        notif_queue.rear->next = node;
        notif_queue.rear = node;
    }
    notif_queue.size++;
}

static void sort_by_priority(void) {
    if (notif_queue.size <= 1) return;
    int swapped;
    do {
        swapped = 0;
        NotifNode *cur = notif_queue.front;
        while (cur && cur->next) {
            NotifNode *nxt = cur->next;
            if (cur->priority < nxt->priority) {
                NotifNode *an = cur->next, *bn = nxt->next;
                NotifNode tmp = *cur;
                *cur = *nxt; *nxt = tmp;
                cur->next = an; nxt->next = bn;
                swapped = 1;
            }
            cur = cur->next;
        }
    } while (swapped);
}

static OfficeAccount *find_office_by_id(int id) {
    OfficeAccount *a = office_list;
    while (a) { if (a->id==id) return a; a=a->next; }
    return NULL;
}

static OfficeAccount *find_office_by_username(const char *uname) {
    OfficeAccount *a = office_list;
    while (a) { if (strcmp(a->username, uname)==0) return a; a=a->next; }
    return NULL;
}

static Citizen *find_by_id(int id) {
    Citizen *c = citizen_list;
    while (c) { if (c->id==id) return c; c=c->next; }
    return NULL;
}

static Citizen *find_by_name(const char *name) {
    Citizen *c = citizen_list;
    while (c) { if (strstr(c->name,name)) return c; c=c->next; }
    return NULL;
}

static NotifNode *find_notif(int id) {
    NotifNode *n = notif_queue.front;
    while (n) { if (n->id==id) return n; n=n->next; }
    return NULL;
}

static OfficeAccount *office_login(void) {
    char uname[MAX_STR], pass[MAX_STR];

    if (office_list == NULL) {
        printf("\n  No office accounts exist yet.\n");
        printf("  The first account will be created as Super Admin.\n\n");

        OfficeAccount *acc = malloc(sizeof(OfficeAccount));
        if (!acc) { printf("  Memory error.\n"); return NULL; }
        memset(acc, 0, sizeof(OfficeAccount));
        acc->id   = oid_counter++;
        acc->role = OFFICE_ROLE_SUPER;
        acc->active = 1;

        printf("  Set Username    : "); fgets(acc->username, MAX_STR, stdin);
        acc->username[strcspn(acc->username,"\n")] = 0;
        if (strlen(acc->username) == 0) { free(acc); printf("  Username cannot be empty.\n"); return NULL; }

        if (find_office_by_username(acc->username)) {
            free(acc);
            printf("  Username already taken.\n");
            return NULL;
        }

        printf("  Set Password    : "); fgets(acc->password, MAX_STR, stdin);
        acc->password[strcspn(acc->password,"\n")] = 0;

        printf("  Office Name     : "); fgets(acc->office_name, MAX_STR, stdin);
        acc->office_name[strcspn(acc->office_name,"\n")] = 0;

        acc->next   = office_list;
        office_list = acc;
        printf("\n  [OK] Super Admin account created! ID = %d\n", acc->id);
        printf("  Please log in.\n\n");
    }

    printf("  Username : "); fgets(uname, MAX_STR, stdin);
    uname[strcspn(uname,"\n")] = 0;
    printf("  Password : "); fgets(pass, MAX_STR, stdin);
    pass[strcspn(pass,"\n")] = 0;

    OfficeAccount *acc = find_office_by_username(uname);
    if (!acc || !acc->active) {
        printf("  Login failed: account not found or inactive.\n");
        return NULL;
    }
    if (strcmp(acc->password, pass) != 0) {
        printf("  Login failed: wrong password.\n");
        return NULL;
    }

    printf("\n  Welcome, %s! [%s | %s]\n", acc->username,
           acc->office_name, office_role_str(acc->role));
    return acc;
}

static void sa_create_account(void) {
    OfficeAccount *acc = malloc(sizeof(OfficeAccount));
    if (!acc) { printf("  Memory error.\n"); return; }
    memset(acc, 0, sizeof(OfficeAccount));
    acc->id     = oid_counter++;
    acc->active = 1;

    printf("  Username    : "); fgets(acc->username, MAX_STR, stdin);
    acc->username[strcspn(acc->username,"\n")] = 0;
    if (strlen(acc->username)==0) { free(acc); printf("  Username cannot be empty.\n"); return; }
    if (find_office_by_username(acc->username)) { free(acc); printf("  Username already exists.\n"); return; }

    printf("  Password    : "); fgets(acc->password, MAX_STR, stdin);
    acc->password[strcspn(acc->password,"\n")] = 0;

    printf("  Office Name : "); fgets(acc->office_name, MAX_STR, stdin);
    acc->office_name[strcspn(acc->office_name,"\n")] = 0;

    printf("  Role [0=Admin 1=Super Admin]: ");
    int r; scanf("%d",&r); clear_stdin();
    acc->role = (r==1) ? OFFICE_ROLE_SUPER : OFFICE_ROLE_ADMIN;

    acc->next   = office_list;
    office_list = acc;
    printf("  [OK] Account created. ID = %d  Role = %s\n", acc->id, office_role_str(acc->role));
}

static void sa_list_accounts(void) {
    printf("\n  %-4s %-20s %-20s %-14s %-8s\n","ID","Username","Office","Role","Active");
    printf("  %s\n","────────────────────────────────────────────────────────────────");
    OfficeAccount *a = office_list;
    while (a) {
        printf("  %-4d %-20s %-20s %-14s %-8s\n",
               a->id, a->username, a->office_name,
               office_role_str(a->role), a->active ? "Yes" : "No");
        a = a->next;
    }
}

static void sa_toggle_account(void) {
    printf("  Enter Account ID to toggle active/inactive: ");
    int id; scanf("%d",&id); clear_stdin();
    OfficeAccount *a = find_office_by_id(id);
    if (!a) { printf("  Account not found.\n"); return; }
    a->active = !a->active;
    printf("  [OK] Account %d (%s) is now %s.\n",
           a->id, a->username, a->active ? "Active" : "Inactive");
}

static void sa_delete_account(OfficeAccount *current) {
    printf("  Enter Account ID to delete: ");
    int id; scanf("%d",&id); clear_stdin();
    if (id == current->id) { printf("  Cannot delete your own account.\n"); return; }

    OfficeAccount *prev = NULL, *a = office_list;
    while (a && a->id != id) { prev = a; a = a->next; }
    if (!a) { printf("  Account not found.\n"); return; }

    if (prev) prev->next = a->next;
    else       office_list = a->next;
    printf("  [OK] Account %d (%s) deleted.\n", a->id, a->username);
    free(a);
}

static void sa_reset_password(void) {
    printf("  Enter Account ID: ");
    int id; scanf("%d",&id); clear_stdin();
    OfficeAccount *a = find_office_by_id(id);
    if (!a) { printf("  Account not found.\n"); return; }

    char np[MAX_STR];
    printf("  New Password: "); fgets(np, MAX_STR, stdin);
    np[strcspn(np,"\n")] = 0;
    strncpy(a->password, np, MAX_STR-1);
    printf("  [OK] Password updated for %s.\n", a->username);
}


static void notif_create(OfficeAccount *acc) {
    NotifNode n;
    memset(&n, 0, sizeof(n));
    n.id = nid_counter++;
    n.active = 1;
    get_time(n.created_at, MAX_STR);
    strncpy(n.office, acc->office_name, MAX_STR-1);

    printf("  Title        : "); fgets(n.title, MAX_STR, stdin);
    n.title[strcspn(n.title,"\n")] = 0;

    printf("  Message      : "); fgets(n.message, MAX_MSG, stdin);
    n.message[strcspn(n.message,"\n")] = 0;

    printf("  Priority [0=Low 1=Normal 2=High 3=Emergency]: ");
    scanf("%d",&n.priority); clear_stdin();
    if (n.priority<0||n.priority>3) n.priority=PRIO_NORMAL;

    printf("  Target [0=All 1=Region 2=Status 3=Education]: ");
    scanf("%d",&n.target_type); clear_stdin();

    if (n.target_type == TGT_REGION) {
        printf("  Region name: "); fgets(n.target_value, MAX_STR, stdin);
        n.target_value[strcspn(n.target_value,"\n")] = 0;
    } else if (n.target_type == TGT_STATUS) {
        printf("  Status [0=Student 1=Worker 2=General]: ");
        int v; scanf("%d",&v); clear_stdin();
        snprintf(n.target_value, MAX_STR, "%d", v);
    } else if (n.target_type == TGT_EDUCATION) {
        printf("  Min Education level [0-5]: ");
        int v; scanf("%d",&v); clear_stdin();
        snprintf(n.target_value, MAX_STR, "%d", v);
    } else {
        strcpy(n.target_value, "all");
    }

    enqueue(n);
    printf("  [OK] Notification created. ID = %d\n", n.id);
}

static void notif_list(OfficeAccount *acc) {
    if (notif_queue.size == 0) { printf("  No notifications.\n"); return; }
    sort_by_priority();

    printf("\n  %-4s %-22s %-15s %-10s %-10s\n","ID","Title","Office","Priority","Target");
    printf("  %s\n","──────────────────────────────────────────────────────────────");
    NotifNode *n = notif_queue.front;
    int shown = 0;
    while (n) {
        if (n->active &&
            (acc->role == OFFICE_ROLE_SUPER || strcmp(n->office, acc->office_name)==0)) {
            printf("  %-4d %-22s %-15s %-10s %-10s\n",
                   n->id, n->title, n->office, prio_str(n->priority), tgt_str(n->target_type));
            shown++;
        }
        n = n->next;
    }
    if (!shown) printf("  No notifications found for your office.\n");
}

static void notif_search(void) {
    printf("  Enter Notification ID: "); int id; scanf("%d",&id); clear_stdin();
    NotifNode *n = find_notif(id);
    if (!n) { printf("  Not found.\n"); return; }
    printf("  ID:%d | %s | Office:%s | Priority:%s | Target:%s\n",
           n->id, n->title, n->office, prio_str(n->priority), tgt_str(n->target_type));
    printf("  Message : %s\n  Date    : %s\n", n->message, n->created_at);
}

static void notif_deactivate(OfficeAccount *acc) {
    printf("  Enter Notification ID to remove: "); int id; scanf("%d",&id); clear_stdin();
    NotifNode *n = find_notif(id);
    if (!n) { printf("  Not found.\n"); return; }
    /* Regular admin can only remove their own office's notifications */
    if (acc->role != OFFICE_ROLE_SUPER && strcmp(n->office, acc->office_name) != 0) {
        printf("  Access denied: you can only remove notifications from your own office.\n");
        return;
    }
    n->active = 0;
    printf("  [OK] Notification %d removed.\n", id);
}

static void citizen_register(void) {
    Citizen *c = malloc(sizeof(Citizen));
    if (!c) { printf("Memory error.\n"); return; }
    c->id = cid_counter++;
    c->next = NULL;

    printf("  Name     : "); fgets(c->name, MAX_STR, stdin);
    c->name[strcspn(c->name,"\n")] = 0;

    printf("  Region   : "); fgets(c->region, MAX_STR, stdin);
    c->region[strcspn(c->region,"\n")] = 0;

    printf("  Status   [0=Student 1=Worker 2=General]: ");
    scanf("%d",&c->status); clear_stdin();
    if (c->status<0||c->status>2) c->status=2;

    printf("  Education[0=Primary 1=O-Level 2=A-Level 3=Diploma 4=Undergrad 5=Postgrad]: ");
    scanf("%d",&c->education); clear_stdin();
    if (c->education<0||c->education>5) c->education=0;

    c->next = citizen_list;
    citizen_list = c;
    printf("  [OK] Registered! Your ID = %d\n", c->id);
}

static void citizen_search(void) {
    printf("  Search by [1] ID  [2] Name: ");
    int ch; scanf("%d",&ch); clear_stdin();

    if (ch == 1) {
        printf("  Enter ID: "); int id; scanf("%d",&id); clear_stdin();
        Citizen *c = find_by_id(id);
        if (!c) { printf("  Not found.\n"); return; }
        printf("  ID:%d | %s | Region:%s | Status:%s | Edu:%s\n",
               c->id, c->name, c->region, status_str(c->status), edu_str(c->education));
    } else {
        printf("  Enter name: "); char nm[MAX_STR];
        fgets(nm, MAX_STR, stdin); nm[strcspn(nm,"\n")]=0;
        Citizen *c = find_by_name(nm);
        if (!c) { printf("  Not found.\n"); return; }
        printf("  ID:%d | %s | Region:%s\n", c->id, c->name, c->region);
    }
}

static int matches(Citizen *c, NotifNode *n) {
    if (n->priority == PRIO_EMERGENCY) return 1;
    switch (n->target_type) {
        case TGT_ALL:       return 1;
        case TGT_REGION:    return strstr(c->region, n->target_value) ? 1 : 0;
        case TGT_STATUS:    return c->status    == atoi(n->target_value);
        case TGT_EDUCATION: return c->education >= atoi(n->target_value);
        default:            return 0;
    }
}

static void view_my_alerts(void) {
    printf("  Enter your Citizen ID: "); int id; scanf("%d",&id); clear_stdin();
    Citizen *c = find_by_id(id);
    if (!c) { printf("  Citizen not found.\n"); return; }

    sort_by_priority();
    printf("\n  Alerts for %s:\n", c->name);
    printf("  %s\n","──────────────────────────────────────────────");
    int found = 0;
    NotifNode *n = notif_queue.front;
    while (n) {
        if (n->active && matches(c, n)) {
            printf("  [%s] %s\n", prio_str(n->priority), n->title);
            printf("  From: %s | %s\n", n->office, n->created_at);
            printf("  %s\n\n", n->message);
            found++;
        }
        n = n->next;
    }
    if (!found) printf("  No alerts for you right now.\n");
}


static void menu_office_super(OfficeAccount *acc) {
    int ch;
    do {
        printf("\n  ╔══════════════════════════════════╗\n");
        printf("  ║  Office Menu  [SUPER ADMIN]      ║\n");
        printf("  ║  -- Notifications --             ║\n");
        printf("  ║  1. Create Notification          ║\n");
        printf("  ║  2. List  Notifications          ║\n");
        printf("  ║  3. Search Notification          ║\n");
        printf("  ║  4. Remove Notification          ║\n");
        printf("  ║  -- Account Management --        ║\n");
        printf("  ║  5. Create Office Account        ║\n");
        printf("  ║  6. List   Office Accounts       ║\n");
        printf("  ║  7. Toggle Account Active        ║\n");
        printf("  ║  8. Delete Office Account        ║\n");
        printf("  ║  9. Reset Account Password       ║\n");
        printf("  ║  0. Logout                       ║\n");
        printf("  ╚══════════════════════════════════╝\n");
        printf("  Choice: ");
        if (scanf("%d",&ch)!=1){clear_stdin();ch=-1;continue;}
        clear_stdin();
        switch(ch) {
            case 1: notif_create(acc);         break;
            case 2: notif_list(acc);           break;
            case 3: notif_search();            break;
            case 4: notif_deactivate(acc);     break;
            case 5: sa_create_account();       break;
            case 6: sa_list_accounts();        break;
            case 7: sa_toggle_account();       break;
            case 8: sa_delete_account(acc);    break;
            case 9: sa_reset_password();       break;
            case 0: printf("  Logged out.\n"); break;
            default: printf("  Invalid choice.\n");
        }
    } while (ch != 0);
}

static void menu_office_admin(OfficeAccount *acc) {
    int ch;
    do {
        printf("\n  ╔══════════════════════════════════╗\n");
        printf("  ║  Office Menu  [%s]       ║\n", acc->office_name);
        printf("  ║  1. Create Notification          ║\n");
        printf("  ║  2. List  Notifications          ║\n");
        printf("  ║  3. Search Notification          ║\n");
        printf("  ║  4. Remove Notification          ║\n");
        printf("  ║  0. Logout                       ║\n");
        printf("  ╚══════════════════════════════════╝\n");
        printf("  Choice: ");
        if (scanf("%d",&ch)!=1){clear_stdin();ch=-1;continue;}
        clear_stdin();
        switch(ch) {
            case 1: notif_create(acc);     break;
            case 2: notif_list(acc);       break;
            case 3: notif_search();        break;
            case 4: notif_deactivate(acc); break;
            case 0: printf("  Logged out.\n"); break;
            default: printf("  Invalid choice.\n");
        }
    } while (ch != 0);
}

static void menu_citizen(void) {
    int ch;
    do {
        printf("\n  ╔══════════════════════════════╗\n");
        printf("  ║  Citizen Portal              ║\n");
        printf("  ║  1. Register                 ║\n");
        printf("  ║  2. Search Citizen           ║\n");
        printf("  ║  3. View My Alerts           ║\n");
        printf("  ║  0. Exit                     ║\n");
        printf("  ╚══════════════════════════════╝\n");
        printf("  Choice: ");
        if (scanf("%d",&ch)!=1){clear_stdin();ch=-1;continue;}
        clear_stdin();
        switch(ch) {
            case 1: citizen_register(); break;
            case 2: citizen_search();   break;
            case 3: view_my_alerts();   break;
            case 0: printf("  Goodbye!\n"); break;
            default: printf("  Invalid choice.\n");
        }
    } while (ch != 0);
}

int main(void) {
    printf("\n  ╔══════════════════════════════════╗\n");
    printf("  ║  CITIZEN ALERT SYSTEM  v5.0      ║\n");
    printf("  ║          Zanzibar                ║\n");
    printf("  ╚══════════════════════════════════╝\n\n");

    int role;

    while (1) {
        printf("\n  Login as: [0] Office Admin   [1] Citizen   [-1] Quit\n");
        printf("  Choice: ");

        if (scanf("%d",&role)!=1) {
            clear_stdin();
            continue;
        }
        clear_stdin();

        if (role == -1) break;

        if (role == ROLE_OFFICE) {
            OfficeAccount *acc = office_login();
            if (!acc) continue;
            if (acc->role == OFFICE_ROLE_SUPER)
                menu_office_super(acc);
            else
                menu_office_admin(acc);
        } else {
            menu_citizen();
        }
    }

    OfficeAccount *a = office_list;
    while (a) { OfficeAccount *nx=a->next; free(a); a=nx; }

    Citizen *c = citizen_list;
    while (c) { Citizen *nx=c->next; free(c); c=nx; }

    NotifNode *n = notif_queue.front;
    while (n) { NotifNode *nx=n->next; free(n); n=nx; }

    printf("\nProgram terminated safely.\n");
    return 0;
}
