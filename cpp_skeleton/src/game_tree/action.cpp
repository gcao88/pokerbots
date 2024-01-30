struct Action {
    float r = 0;
    float s = 0.001; 
    float prob = -1; 
    string label;

    Action(string label, float prob) : label(label), prob(prob) {}
};
