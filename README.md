# Introdução

Foram realizadas reuniões e discussões com a equipe da Polícia Militar de Alagoas para entender o método de patrulhamento e desenvolver uma modelagem matemática satisfatória para o problema. A definição dos pontos quentes foi modelada de acordo com o trabalho de Koper (1995), onde cada ponto quente é centrado em uma esquina e sua pontuação cobre as pontuações de todas as esquinas visíveis e em menos de 250 metros de raio. A definição do subconjunto de pontos quentes se dá pelo problema de Conjunto Dominante.

Sejam $m$ o número de viaturas, $T_{prot}$ o tempo de proteção residual e $L$ o tempo limite de cada rota, cada uma das viaturas deve obedecer o seguinte:

1. As rotas não devem exceder o tempo $L$.
2. Nenhum vértice pode ser revisitado em um intervalo menor que $T_{prot}$, nem por uma viatura distinta.
3. Cada viatura pode passar ou parar por 15 minutos, recebendo recompensa maior em caso de paradas.

O Orienteering Problem (OP) foi provado ser $\mathcal{NP\text{-difícil}}$ por Golden et al. (1987), por extensão, o problema de design de rotas da PMAL é $\mathcal{NP\text{-difícil}}$.

# Como Executar

## Pré-requisitos

1. Certifique-se de ter o CPLEX instalado e configurado corretamente no seu sistema.
2. Tenha o compilador `g++` instalado.

## Compilação

Antes de compilar o programa, ajuste o caminho do CPLEX no arquivo `Makefile` conforme necessário. Atualize a variável `CPLEXPATH` para apontar para o diretório correto onde o CPLEX está instalado.

Exemplo de ajuste no `Makefile`:
```makefile
CPLEXPATH = /<caminho_para_cplex>
```

## Entrada

-   $G = (V, E)$ um grafo direcionado completo onde cada vértice
    $v \in V$ representa um esquina da cidade de Maceió. Cada vértice de
    $G$ tem um prêmio em função do instante que é visitado, os hot spots
    serão definidos como um subconjunto $V' \subset V$ de maneira que
    $V'$ é um Conjunto Dominante de $G$, dessa forma, o problema será
    otimizar as rotas para o grafo $G'$ completo induzido de $V'$. Sendo
    assim, o objetivo do problema é maximizar a soma dos prêmios
    coletados em cada vértice.

-   Depósitos: 0 e $f=n\phi$

-   $V=\{0, 1, \dots, n\phi +1 \}$

-   $V^o=\{0*\phi+1, 1\phi +1, \dots, n\phi +1 \}$

-   $V_i=\{v | i  \in V^o; i < v < i+ \phi \}$

-   $N=\{1, \dots, n\phi \}$

## Variáveis

As famílias de variáveis são:

-   $x_{uv}$: binária que indica se o arco $uv$ faz parte da solução.

-   $y_v$: binária que indica se o vértice $v$ foi utilizado.

-   $s_v$: binária que indica se o vértice $v$ foi um ponto de parada na
    solução.

-   $z_{uv}$: O tempo de chegada no vértice $v$ se ele partiu do vértice
    $u$, 0 caso o contrário.

## Formulação matemática

$$\begin{aligned}
    \max \quad & \sum_{v\in N}  \{ \sigma \cdot y_v + (1 - \sigma) \cdot s_v\} \\
    & \sum_{v \in N} x_{0v} = \sum_{v \in N} x_{v,f} = m \\
    & \sum_{uv \in \delta^-(v)} x_{uv} = \sum_{vu \in \delta^+(v)} x_{vu} = y_v && \forall v \in N \\
    & z_{0v} = t_{0v} x_{0v} && \forall v \in N \\
    & \sum_{a \in \delta+(v)} {z_{a}} - \sum_{a \in \delta-(v)} {z_{a}} = \sum_{a \in \delta+(v)} {t_{a} x_{a}} + T_{parada} \cdot s_v && \forall v \in N \\
    & z_{vu} \le T_{u,f}^{\max} x_{vu} && vu \in A - \{ (0, f) \} \\
    & z_{vu} \ge t_{vu}^0 x_{vu} && vu \in A - \{ (0, f) \} \\
    & 0 \le x_{0, f} \le m \\
    & y_{v + i} \leq y_{v + i - 1} && \forall v \in V^o, \ i=1,...,\phi-1\\
    & \sum_{a \in \delta^-(v + i)} z_a + M(1 - y_{v + i}) \ge \sum_{a \in \delta^-(v + i-1)} z_a + T_{prot} + s_{v + i - 1} \cdot T_{parada} && \forall v \in V^0,  \ i=1,...,\phi-1 \\
    & s_v \le y_v && \forall v \in V \\
    & y_v \in \{ 0, 1 \} && v \in N \\
    & s_v \in \{ 0, 1 \} && v \in N \\
    & x_{vu} \in \{ 0, 1 \} && vu \in A - \{ (0, f)\} 
\end{aligned}$$

# Formato da Instância

O arquivo de instância deve conter os seguintes valores, um por linha:

1. **Número de pontos (`n`)**: O número total de pontos que precisam ser patrulhados.
2. **Número de viaturas (`m`)**: O número de viaturas disponíveis para patrulhamento.
3. **Tempo limite de cada rota (`L`)**: O tempo máximo permitido para cada rota de patrulhamento.
4. **Tempo de proteção residual (`T_prot`)**: O tempo mínimo que deve passar antes que um ponto possa ser revisitado.
5. **Velocidade das viaturas (`velocidade`)**: A velocidade com que as viaturas se movem.