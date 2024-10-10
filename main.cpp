#include <ilcplex/ilocplex.h>
#include <vector>
#include <algorithm>

using namespace std;

int transf(int v, int phi) {
    if(v == 0) return 0;
    return (v-1)/phi + 1;
}

int32_t main() {
    IloEnv env;
    IloModel TOP(env, "Team OP");
    IloCplex CPLEX(TOP);

    // entrada 
    int n; cin >> n;
    int m; cin >> m;
    double L; cin >> L;
    L = L / 60;
    double T_prot;
    cin >> T_prot;
    double velocidade;
    cin >> velocidade;

    int phi = floor((double) L / T_prot);


    double t_parada = 15;
    double sigma = 1.0/3;
    double tr_coef = 1/((velocidade/60)*1000);

    const int f = n*phi + 1;

    vector<int> p(n + 1);
    for(int i = 0; i < n; i++) {
        int v; cin >> v;
        cin >> p[v];
    }
    p[n] = p[0];

    vector<vector<double>> dist(n + 1, vector<double>(n + 1));
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            double distance;
            cin >> distance;
            dist[i][j] = distance * tr_coef;
        }
    }
    // ja que 0 == n	
    for(int i = 0; i < n; i++) {
        dist[n][i] = dist[0][i];
        dist[i][n] = dist[i][0];
    }
    dist[n][n] = 0;

    vector<IloIntVar*> y(f+1, nullptr);
	vector<IloIntVar*> s(f+1, nullptr);
	vector<vector<IloIntVar*>> x(f+1, vector<IloIntVar*>(f+1, nullptr));
	vector<vector<IloNumVar*>> z(f+1, vector<IloNumVar*>(f+1, nullptr));

    //1
    {
        IloExpr sum(env);
        for(int v = 1; v <= n*phi; v++) {
            if(y[v] == nullptr) y[v] = new IloIntVar(env, 0, 1);
            if(s[v] == nullptr) s[v] = new IloIntVar(env, 0, 1);
            sum += sigma*(*y[v]) + (1 - sigma)*(*s[v]);
        }
        TOP.add(IloMaximize(env, sum));
    }

    //2
    {
        IloExpr sum1(env), sum2(env);
        for(int v = 1; v <= n*phi; v++) {

            if(x[0][v] == nullptr) x[0][v] = new IloIntVar(env, 0, 1);
            if(x[v][f] == nullptr) x[v][f] = new IloIntVar(env, 0, 1);
            sum1 += (*x[0][v]);
            sum2 += (*x[v][f]);
        }
        TOP.add(sum1 == m);
        TOP.add(sum2 == m);
    }

    //3
    {
        for(int v = 1; v <= n*phi; v++) {
            IloExpr sum1(env), sum2(env);
            //vizinhos neg
            for(int u = 0; u <= n*phi; u++) {
                if(u == v) continue;
                if(x[u][v] == nullptr) x[u][v] = new IloIntVar(env, 0, 1);
                sum1 += (*x[u][v]);
            }

            //vizinhos pos
            for(int u = 1; u <= n*phi + 1; u++) {
                if(u == v) continue;
                if(x[v][u] == nullptr) x[v][u] = new IloIntVar(env, 0, 1);
                sum1 += (*x[v][u]);
            }

            if(y[v] == nullptr) y[v] = new IloIntVar(env, 0, 1);
            TOP.add(sum1 == (*y[v]));
            TOP.add(sum2 == (*y[v]));
        }
    }

    //4
    {
        for(int v = 1; v <= n*phi; v++) {
            int v_transf = transf(v, phi);
            if(z[0][v] == nullptr) z[0][v] = new IloNumVar(env, 0, L);
            if(x[0][v] == nullptr) x[0][v] == new IloIntVar(env, 0, 1);
            TOP.add(*z[0][v] == dist[0][v_transf] * (*x[0][v]));
        }
    }

    //5
    {
        for(int v = 1; v <= n*phi; v++) {
            IloExpr sum1(env), sum2(env), sum3(env);
            //vizinhos pos
            for(int u = 1; u <= n*phi + 1; u++) {
                int v_transf = transf(v, phi), u_transf = transf(u, phi);
                if(u == v) continue;
                if(z[v][u] == nullptr) z[v][u] = new IloNumVar(env, 0, L);
                if(x[v][u] == nullptr) x[v][u] = new IloIntVar(env, 0, 1);
                sum1 += (*z[v][u]);
                sum3 += (dist[v_transf][u_transf] * (*x[v][u]));
            }

            //vizinhos neg
            for(int u = 0; u <= n*phi; u++) {
                if(u == v) continue;
                if(z[u][v] == nullptr) z[u][v] = new IloNumVar(env, 0, L);
                sum1 += (*z[u][v]);
            }
            if(s[v] == nullptr) s[v] = new IloIntVar(env, 0, 1);
            TOP.add(sum1 - sum2 == sum3 + t_parada*(*s[v]));
        }
    }

    //6
    {
        for(int u = 0; u <= n*phi; u++) {
            for(int v = 1; v <= n*phi + 1; v++) {
                if(u == v) continue;
                if(u == 0 && v == f) continue;
                int v_transf = transf(v, phi), u_transf = transf(u, phi);
                if(z[u][v] == nullptr) z[u][v] = new IloNumVar(env, 0, L);
				if(x[u][v] == nullptr) x[u][v] = new IloIntVar(env, 0, 1);
                TOP.add(*(z[u][v]) <= (L - dist[v_transf][n])* *(x[u][v]));
            }
        }
    }
    /*
    

    //7
    {
        for(int v = 0; v <= n*phi; v++) {
            for(int u = 1; u <= n*phi + 1; u++) {
                if(u != v) continue;
                if(u == 0 && v == f) continue;
                int v_transf = transf(v, phi), u_transf = transf(u, phi);
				if(z[v][u] == nullptr) z[v][u] = new IloNumVar(env, 0, L); 
                if(x[v][u] == nullptr) x[v][u] = new IloIntVar(env, 0, 1);
				TOP.add((*z[v][u]) >= (dist[0][u_transf] + dist[v_transf][u_transf]) * (*x[v][u]));
            }
        }
    }

    //8
    {
        x[0][f] = new IloIntVar(env, 0, m);
    }

    //9
    {
        for(int v = 0; v <= n*phi + 1; v += phi) {
            for(int i = 1; i <= phi - 1 && v + i <= f; i++) {
                if(y[v + i] == nullptr) y[v + i] = new IloIntVar(env, 0, 1);
                if(y[v + i - 1] == nullptr) y[v + i - 1] = new IloIntVar(env, 0, 1);
                TOP.add(*y[v + i] <= *y[v + i - 1]);
            }
        }
    }

    //10
    {
        IloExpr sum1(env), sum2(env);
        for(int v = 0; v <= n*phi + 1; v += phi) {
            for(int i = 1; i <= phi - 1 && v + i <= f; i++) {
                IloExpr sum1(env), sum2(env);
                for(int u = 0; u <= n*phi; u++) {
                    if(u == v + i) continue;
                    if(z[u][v + i] == nullptr) z[u][v + i] = new IloNumVar(env, 0, L);
                    sum1 += *z[u][v + i];
                }
                for(int u = 0; u <= n*phi; u++) {
                    if(u == v + i - 1) continue;
                    if(z[u][v + i - 1] == nullptr) z[u][v + i - 1] = new IloNumVar(env, 0, L);
                    sum2 += *z[u][v + i - 1];
                }
                const int M = L + T_prot;
                if(y[v + i] == nullptr) y[v + i] = new IloIntVar(env, 0, 1);
                TOP.add(sum1 + M*(1 - *y[v + i]) >= sum2 + T_prot);
            }
        }
    }


    //11
    {
        for(int v = 1; v <= n*phi; v++) {
            if(s[v] == nullptr) s[v] = new IloIntVar(env, 0, 1);
            if(y[v] == nullptr) y[v] = new IloIntVar(env, 0, 1);
            TOP.add(*s[v] <= *y[v]);
        }
    }

    
    */
    CPLEX.exportModel("modelo.lp");
	if ( CPLEX.solve() ) {
        cout << "oi" << endl;
		cerr << "Premio ótimo: " << CPLEX.getObjValue() << endl;
	}
}