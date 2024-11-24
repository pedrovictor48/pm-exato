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

Antes de compilar o programa, ajuste o caminho do CPLEX no arquivo `Makefile` conforme necessário. Atualize as variáveis `CPLEXFLAGS` para apontar para o diretório correto onde o CPLEX está instalado.

Exemplo de ajuste no `Makefile`:
```makefile
CPLEXFLAGS=-O3 -m64 -O -fPIC -fexceptions -DNDEBUG -DIL_STD -I/<caminho_para_cplex>/cplex/include -I/<caminho_para_cplex>/concert/include  -L/<caminho_para_cplex>/cplex/lib/x86-64_linux/static_pic -lilocplex -lcplex -L/<caminho_para_cplex>/concert/lib/x86-64_linux/static_pic -lconcert -lm -pthread -std=c++0x -ldl
```

# Formato da Instância

O arquivo de instância deve conter os seguintes valores, um por linha:

1. **Número de pontos (`n`)**: O número total de pontos que precisam ser patrulhados.
2. **Número de viaturas (`m`)**: O número de viaturas disponíveis para patrulhamento.
3. **Tempo limite de cada rota (`L`)**: O tempo máximo permitido para cada rota de patrulhamento.
4. **Tempo de proteção residual (`T_prot`)**: O tempo mínimo que deve passar antes que um ponto possa ser revisitado.
5. **Velocidade das viaturas (`velocidade`)**: A velocidade com que as viaturas se movem.