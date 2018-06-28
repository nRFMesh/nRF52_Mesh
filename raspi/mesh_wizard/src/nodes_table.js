


class NodesTable {
    static addNodeToTable(name,nodeid){
        var text_to_add = '<button id=n'+nodeid+' type="button" class="btn btn-primary">'+name+' '+'<span class="badge badge-light">'+nodeid+'</span></button></br>';
        //$(".pre-scrollable").append(text_to_add);
        $("#NodesTable").append(text_to_add);
        console.log("Added hello");
    }
    
    static removeNodeFromTable(nodeid){
        $("#n"+nodeid).remove();
    }
}

export {NodesTable};
