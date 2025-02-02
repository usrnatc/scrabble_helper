#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "getopt.h"

int optind = 1;
int opterr = 1;
int optopt = '?';

enum ENUM_ORDERING {
    REQUIRE_ORDER,
    PERMUTE,
    RETURN_IN_ORDER
};

// better string functions---------------------------------------------------
#include <immintrin.h>
#include <stdint.h>
#include <intrin.h>
#include <assert.h>

size_t
sstrlen(const char* Str)
{
	const __m512i V01 = _mm512_set1_epi32(0x01010101U);
    const __m512i V80 = _mm512_set1_epi32(0x80808080U);
    size_t Result = 0;

    for (;;) {
        const __m512i V = _mm512_loadu_si512(Str + Result);
        const __m512i V1 = _mm512_sub_epi32(V, V01);
        const __m512i HasZero = _mm512_ternarylogic_epi32(V1, V, V80, 0x20);
        const __mmask16 Mask = _mm512_test_epi32_mask(HasZero, HasZero);

        if (Mask) {
            const size_t N = __lzcnt(Mask);

            if (!Str[Result + 4 * N + 0])
                return Result + 4 * N + 0;

            if (!Str[Result + 4 * N + 1])
                return Result + 4 * N + 1;

            if (!Str[Result + 4 * N + 2])
                return Result + 4 * N + 2;

            assert(!Str[Result + 4 * N + 3]);

            return Result + 4 * N + 3;
        }

        Result += 64;
    }
}

__forceinline __mmask16 zero_byte_mask(const __m512i V)
{
	const __m512i V01 = _mm512_set1_epi32(0x01010101U);
	const __m512i V80 = _mm512_set1_epi32(0x80808080U);
	const __m512i V1 = _mm512_sub_epi32(V, V01);
	const __m512i Tmp1 = _mm512_ternarylogic_epi32(V1, V, V80, 0x20);
	
	return _mm512_test_epi32_mask(Tmp1, Tmp1);
}

int
sstrcmp(const char* Str1, const char* Str2)
{
	char* C1 = (char*) Str1;
    char* C2 = (char*) Str2;

    __mmask16 Zero1;
    __mmask16 Diff;
    __mmask16 Either;

    for (;;) {
        const __m512i V1 = _mm512_loadu_si512(C1);
        const __m512i V2 = _mm512_loadu_si512(C2);

        Zero1 = zero_byte_mask(V1);
        Diff  = _mm512_cmpneq_epi32_mask(V1, V2);
		Either = Zero1 | Diff;

        if (Either)
            break;

        C1 += 64;
        C2 += 64;
    }

    const size_t N = __lzcnt(Either);

    C1 += N * 4;
    C2 += N * 4;
    
    for (int I = 0; I < 4; ++I) {
        int A = C1[I];
        int B = C2[I];

        if (A != B || !A) {
            return A - B;
        }
    }

    assert(0);
}

int
sstrncmp(const char* Str1, const char* Str2, size_t N)
{
	char* C1 = (char*) Str1;
	char* C2 = (char*) Str2;
	__mmask16 Zero1;
	__mmask16 Diff;
	__mmask16 Either;
	size_t I = 0;

	while (I < N) {
		const __m512i V1 = _mm512_loadu_si512(C1);
		const __m512i V2 = _mm512_loadu_si512(C2);

		Zero1 = zero_byte_mask(V1);
		Diff = _mm512_cmpneq_epi32_mask(V1, V2);
		Either = Zero1 | Diff;

		if (Either)
			break;

		C1 += 64;
		C2 += 64;
		I += 64;
	}

	size_t Remaining = N - I;

	if (Remaining > 0) {
		const size_t N_Chunks = Remaining / 4;

		for (size_t J = 0; J < N_Chunks; ++J) {
			int A = C1[J];
			int B = C2[J];

			if (A != B || !A)
				return A - B;
		}
	}

	return 0;
}

// TODO(nathan): sstrchr

// better string functions---------------------------------------------------

static struct _getopt_data_a
{
	int optind;
	int opterr;
	int optopt;
	char *optarg;
	int __initialized;
	char *__nextchar;
	enum ENUM_ORDERING __ordering;
	int __posixly_correct;
	int __first_nonopt;
	int __last_nonopt;
} getopt_data_a;

char *optarg;

static void exchange_a(char **argv, struct _getopt_data_a *d)
{
	int bottom = d->__first_nonopt;
	int middle = d->__last_nonopt;
	int top = d->optind;
	char *tem;
	while (top > middle && middle > bottom)
	{
		if (top - middle > middle - bottom)
		{
			int len = middle - bottom;
			int i;
			for (i = 0; i < len; i++)
			{
				tem = argv[bottom + i];
				argv[bottom + i] = argv[top - (middle - bottom) + i];
				argv[top - (middle - bottom) + i] = tem;
			}
			top -= len;
		}
		else
		{
			int len = top - middle;
			int i;
			for (i = 0; i < len; i++)
			{
				tem = argv[bottom + i];
				argv[bottom + i] = argv[middle + i];
				argv[middle + i] = tem;
			}
			bottom += len;
		}
	}
	d->__first_nonopt += (d->optind - d->__last_nonopt);
	d->__last_nonopt = d->optind;
}

static const char *
_getopt_initialize_a (const char *optstring, struct _getopt_data_a *d, int posixly_correct)
{
	d->__first_nonopt = d->__last_nonopt = d->optind;
	d->__nextchar = NULL;
	d->__posixly_correct = posixly_correct | !!getenv("POSIXLY_CORRECT");

	if (optstring[0] == '-') {
		d->__ordering = RETURN_IN_ORDER;
		++optstring;
	} else if (optstring[0] == '+') {
		d->__ordering = REQUIRE_ORDER;
		++optstring;
	} else if (d->__posixly_correct) {
		d->__ordering = REQUIRE_ORDER;
	} else {
		d->__ordering = PERMUTE;
	}

	return optstring;
}

int 
_getopt_internal_r_a(
	int argc, 
	char *const *argv, 
	const char *optstring, 
	const struct option_a *longopts, 
	int *longind, 
	int long_only, 
	_getopt_data_a *d, 
	int posixly_correct
) {
	int print_errors = d->opterr;

	if (argc < 1)
		return -1;

	d->optarg = NULL;

	if (d->optind == 0 || !d->__initialized) {
		if (d->optind == 0)
			d->optind = 1;

		optstring = _getopt_initialize_a(optstring, d, posixly_correct);
		d->__initialized = 1;
	} else if (optstring[0] == '-' || optstring[0] == '+') {
		++optstring;
	}

	if (optstring[0] == ':')
		print_errors = 0;

	if (d->__nextchar == NULL || *d->__nextchar == '\0') {
		if (d->__last_nonopt > d->optind)
			d->__last_nonopt = d->optind;

		if (d->__first_nonopt > d->optind)
			d->__first_nonopt = d->optind;

		if (d->__ordering == PERMUTE) {
			if (d->__first_nonopt != d->__last_nonopt && d->__last_nonopt != d->optind)
				exchange_a((char **) argv, d);
			else if (d->__last_nonopt != d->optind)
				d->__first_nonopt = d->optind;

			while (d->optind < argc && (argv[d->optind][0] != '-' || argv[d->optind][1] == '\0'))
				++d->optind;

			d->__last_nonopt = d->optind;
		}

		if (d->optind != argc && !sstrcmp(argv[d->optind], "--")) {
			d->optind++;
			if (d->__first_nonopt != d->__last_nonopt && d->__last_nonopt != d->optind)
				exchange_a((char **) argv, d);
			else if (d->__first_nonopt == d->__last_nonopt)
				d->__first_nonopt = d->optind;
			d->__last_nonopt = argc;
			d->optind = argc;
		}
		if (d->optind == argc)
		{
			if (d->__first_nonopt != d->__last_nonopt)
				d->optind = d->__first_nonopt;
			return -1;
		}
		if ((argv[d->optind][0] != '-' || argv[d->optind][1] == '\0'))
		{
			if (d->__ordering == REQUIRE_ORDER)
				return -1;
			d->optarg = argv[d->optind++];
			return 1;
		}
		d->__nextchar = (argv[d->optind] + 1 + (longopts != NULL && argv[d->optind][1] == '-'));
	}
	if (longopts != NULL && (argv[d->optind][1] == '-' || (long_only && (argv[d->optind][2] || !strchr(optstring, argv[d->optind][1])))))
	{
		char *nameend;
		unsigned int namelen;
		const struct option_a *p;
		const struct option_a *pfound = NULL;
		struct option_list
		{
			const struct option_a *p;
			struct option_list *next;
		} *ambig_list = NULL;
		int exact = 0;
		int indfound = -1;
		int option_index;
		for (nameend = d->__nextchar; *nameend && *nameend != '='; nameend++);
		namelen = (unsigned int)(nameend - d->__nextchar);
		for (p = longopts, option_index = 0; p->name; p++, option_index++)
			if (!sstrncmp(p->name, d->__nextchar, namelen))
			{
				if (namelen == (unsigned int)sstrlen(p->name))
				{
					pfound = p;
					indfound = option_index;
					exact = 1;
					break;
				}
				else if (pfound == NULL)
				{
					pfound = p;
					indfound = option_index;
				}
				else if (long_only || pfound->has_arg != p->has_arg || pfound->flag != p->flag || pfound->val != p->val)
				{
					struct option_list *newp = (option_list*) alloca(sizeof(*newp));
					newp->p = p;
					newp->next = ambig_list;
					ambig_list = newp;
				}
			}
			if (ambig_list != NULL && !exact)
			{
				if (print_errors)
				{
					struct option_list first;
					first.p = pfound;
					first.next = ambig_list;
					ambig_list = &first;
					fprintf(stderr, "%s: option '%s' is ambiguous; possibilities:", argv[0], argv[d->optind]);
					do
					{
						fprintf (stderr, " '--%s'", ambig_list->p->name);
						ambig_list = ambig_list->next;
					}
					while (ambig_list != NULL);
					fputc ('\n', stderr);
				}
				d->__nextchar += sstrlen(d->__nextchar);
				d->optind++;
				d->optopt = 0;
				return '?';
			}
			if (pfound != NULL)
			{
				option_index = indfound;
				d->optind++;
				if (*nameend)
				{
					if (pfound->has_arg)
						d->optarg = nameend + 1;
					else
					{
						if (print_errors)
						{
							if (argv[d->optind - 1][1] == '-')
							{
								fprintf(stderr, "%s: option '--%s' doesn't allow an argument\n",argv[0], pfound->name);
							}
							else
							{
								fprintf(stderr, "%s: option '%c%s' doesn't allow an argument\n",argv[0], argv[d->optind - 1][0],pfound->name);
							}
						}
						d->__nextchar += sstrlen(d->__nextchar);
						d->optopt = pfound->val;
						return '?';
					}
				}
				else if (pfound->has_arg == 1)
				{
					if (d->optind < argc)
						d->optarg = argv[d->optind++];
					else
					{
						if (print_errors)
						{
							fprintf(stderr,"%s: option '--%s' requires an argument\n",argv[0], pfound->name);
						}
						d->__nextchar += sstrlen(d->__nextchar);
						d->optopt = pfound->val;
						return optstring[0] == ':' ? ':' : '?';
					}
				}
				d->__nextchar += sstrlen(d->__nextchar);
				if (longind != NULL)
					*longind = option_index;
				if (pfound->flag)
				{
					*(pfound->flag) = pfound->val;
					return 0;
				}
				return pfound->val;
			}
			if (!long_only || argv[d->optind][1] == '-' || strchr(optstring, *d->__nextchar) == NULL)
			{
				if (print_errors)
				{
					if (argv[d->optind][1] == '-')
					{
						fprintf(stderr, "%s: unrecognized option '--%s'\n",argv[0], d->__nextchar);
					}
					else
					{
						fprintf(stderr, "%s: unrecognized option '%c%s'\n",argv[0], argv[d->optind][0], d->__nextchar);
					}
				}
				d->__nextchar = (char *)"";
				d->optind++;
				d->optopt = 0;
				return '?';
			}
	}
	{
		char c = *d->__nextchar++;
		char *temp = (char*)strchr(optstring, c);
		if (*d->__nextchar == '\0')
			++d->optind;
		if (temp == NULL || c == ':' || c == ';')
		{
			if (print_errors)
			{
				fprintf(stderr, "%s: invalid option -- '%c'\n", argv[0], c);
			}
			d->optopt = c;
			return '?';
		}
		if (temp[0] == 'W' && temp[1] == ';')
		{
			char *nameend;
			const struct option_a *p;
			const struct option_a *pfound = NULL;
			int exact = 0;
			int ambig = 0;
			int indfound = 0;
			int option_index;
			if (longopts == NULL)
				goto no_longs;
			if (*d->__nextchar != '\0')
			{
				d->optarg = d->__nextchar;
				d->optind++;
			}
			else if (d->optind == argc)
			{
				if (print_errors)
				{
					fprintf(stderr,"%s: option requires an argument -- '%c'\n",argv[0], c);
				}
				d->optopt = c;
				if (optstring[0] == ':')
					c = ':';
				else
					c = '?';
				return c;
			}
			else
				d->optarg = argv[d->optind++];
			for (d->__nextchar = nameend = d->optarg; *nameend && *nameend != '='; nameend++);
			for (p = longopts, option_index = 0; p->name; p++, option_index++)
				if (!strncmp(p->name, d->__nextchar, nameend - d->__nextchar))
				{
					if ((unsigned int) (nameend - d->__nextchar) == sstrlen(p->name))
					{
						pfound = p;
						indfound = option_index;
						exact = 1;
						break;
					}
					else if (pfound == NULL)
					{
						pfound = p;
						indfound = option_index;
					}
					else if (long_only || pfound->has_arg != p->has_arg || pfound->flag != p->flag || pfound->val != p->val)
						ambig = 1;
				}
				if (ambig && !exact)
				{
					if (print_errors)
					{
						fprintf(stderr, "%s: option '-W %s' is ambiguous\n",argv[0], d->optarg);
					}
					d->__nextchar += sstrlen(d->__nextchar);
					d->optind++;
					return '?';
				}
				if (pfound != NULL)
				{
					option_index = indfound;
					if (*nameend)
					{
						if (pfound->has_arg)
							d->optarg = nameend + 1;
						else
						{
							if (print_errors)
							{
								fprintf(stderr, "%s: option '-W %s' doesn't allow an argument\n",argv[0], pfound->name);
							}
							d->__nextchar += sstrlen(d->__nextchar);
							return '?';
						}
					}
					else if (pfound->has_arg == 1)
					{
						if (d->optind < argc)
							d->optarg = argv[d->optind++];
						else
						{
							if (print_errors)
							{
								fprintf(stderr, "%s: option '-W %s' requires an argument\n",argv[0], pfound->name);
							}
							d->__nextchar += sstrlen(d->__nextchar);
							return optstring[0] == ':' ? ':' : '?';
						}
					}
					else
						d->optarg = NULL;
					d->__nextchar += sstrlen(d->__nextchar);
					if (longind != NULL)
						*longind = option_index;
					if (pfound->flag)
					{
						*(pfound->flag) = pfound->val;
						return 0;
					}
					return pfound->val;
				}
no_longs:
				d->__nextchar = NULL;
				return 'W';
		}
		if (temp[1] == ':')
		{
			if (temp[2] == ':')
			{
				if (*d->__nextchar != '\0')
				{
					d->optarg = d->__nextchar;
					d->optind++;
				}
				else
					d->optarg = NULL;
				d->__nextchar = NULL;
			}
			else
			{
				if (*d->__nextchar != '\0')
				{
					d->optarg = d->__nextchar;
					d->optind++;
				}
				else if (d->optind == argc)
				{
					if (print_errors)
					{
						fprintf(stderr,"%s: option requires an argument -- '%c'\n",argv[0], c);
					}
					d->optopt = c;
					if (optstring[0] == ':')
						c = ':';
					else
						c = '?';
				}
				else
					d->optarg = argv[d->optind++];
				d->__nextchar = NULL;
			}
		}
		return c;
	}
}

int 
_getopt_internal_a (int argc, char *const *argv, const char *optstring, const struct option_a *longopts, int *longind, int long_only, int posixly_correct)
{
	int result;
	getopt_data_a.optind = optind;
	getopt_data_a.opterr = opterr;
	result = _getopt_internal_r_a (argc, argv, optstring, longopts,longind, long_only, &getopt_data_a,posixly_correct);
	optind = getopt_data_a.optind;
	optarg = getopt_data_a.optarg;
	optopt = getopt_data_a.optopt;
	return result;
}

int 
getopt(int argc, char *const *argv, const char *optstring) throw()
{
	return _getopt_internal_a (argc, argv, optstring, (const struct option_a *) 0, (int *) 0, 0, 0);
}