typedef struct Node_ {
	struct Node_ *next;
	int data;
} LNode;

typedef LNode* List;

List empty()
{
	return NULL;
}

List cons(List l, int d)
{
	LNode *n = GC_MALLOC(sizeof(LNode) * 1);
	n->data = d;
	if (l != NULL) {
		n->next = l;
	}
	return n;
}

List buildList(int n)
{
	int i;
	List l = empty();
	for (i = 0; i < n; i++) {
		l = cons(l, i);
	}
}

int foldList(List l)
{
	int sum = 0;
	while (l != NULL) {
		sum += l->data;
		l = l->next;
	}
	return sum;
}

int buildSmall(int n)
{
	int i, sum, total;
	List l;
	total = 0;

	for (i = 0; i < n; i++) {
		l = buildList(i * 10 + 1);
		sum = foldList(l);
		total += sum;
		// printf("Sum of list %3d was: %5d\n", i, sum);
	}

	return total;
}

void runLL()
{
	buildSmall(10000);
}

